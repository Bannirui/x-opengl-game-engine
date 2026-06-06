//
// Created by rui ding on 2026/6/1.
// IBL (Image-Based Lighting) 环境贴图预计算
//
// 整个 PBR 渲染管线依赖三张预计算的纹理:
//   1. Irradiance Map     — 漫反射 IBL  (低分辨率 32×32, 用半球积分卷积天空盒)
//   2. Prefilter Map      — 镜面反射 IBL (128×128 带 mipmap, 每级 mip 对应不同 roughness)
//   3. BRDF LUT           — BRDF 积分查找表 (512×512 RG16F, 2D 纹理)
//
// 预计算流程:
//   SetEnvironmentMap(cubemap)
//       → BakeIrradiance(cubemap)     — 从 cubemap 采样半球, 生成漫反射辐照度图
//       → BakePrefilter(cubemap)      — 重要性采样 GGX, 生成多 mip 预过滤图
//       → BakeBRDFLUT()               — Monte Carlo 积分, 生成 BRDF 查找表
//
// 共享的 GPU 资源 (s_captureFBO/RBO/VAOs) 在 InitCaptureResources 中创建,
// 三个 Bake 函数复用同一个 FBO + RBO, 通过 glFramebufferTexture2D 切换渲染目标
//

#include "pbr_env.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

// =============================================================================
// Cubemap 捕获矩阵
// =============================================================================

// 捕获投影矩阵 (90°垂直FOV, 用于渲染 cubemap 的每个面)
//   - 列主序: 16 个 float
//   - m[1][1] = -1.0: 翻转 Y 轴 (OpenGL 纹理坐标系 Y 朝上)
//   - m[2][2] = -0.5: 近平面 0.1, 远平面 10.0 的简化正交/透视混合
static constexpr float CUBEMAP_CAPTURE_PROJECTION[16] = {
    1.0f, 0.0f,  0.0f,  0.0f,  //
    0.0f, -1.0f, 0.0f,  0.0f,  //
    0.0f, 0.0f,  -0.5f, 0.0f,  //
    0.0f, 0.0f,  0.0f,  1.0f  //
};

// Cubemap 6 个面的视图矩阵
//   顺序: +X, -X, +Y, -Y, +Z, -Z  (对应 GL_TEXTURE_CUBE_MAP_POSITIVE_X 等枚举)
//   lookAt(原点, 看向方向, up向量)
//   up 向量用 -Y 方向使采样的天空盒朝向与渲染时一致
static const glm::mat4 CAPTURE_VIEWS[6] = {
    glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),  // +X: look right
    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),  // -X: look left
    glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),  // +Y: look up
    glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),  // -Y: look down
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),  // +Z: look forward
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)),  // -Z: look backward
};

// 帧缓冲对象 渲染目标切换用
GLuint PBREnvironment::s_captureFBO = 0;
// 深度渲染缓冲 按mip级别动态调整大小
GLuint PBREnvironment::s_captureRBO = 0;
// 单位立方体VAO 36顶点 用于irradiance/prefilter
GLuint PBREnvironment::s_cubeVAO = 0;
// 全屏四边形VAO 4顶点 用于BRDF LUT
GLuint PBREnvironment::s_quadVAO = 0;

// =============================================================================
// compileShader() — 编译 GLSL 源码为 OpenGL Program (不经过引擎的 SPIRV 管线)
//
// 为什么不用引擎的 Shader::Create():
//   这些是内部工具 shader, 不需要缓存, 不需要反射, 用完即删
//   直接 glCompileShader + glLinkProgram 比 SPIRV 路径更快更简单
//   且与引擎主渲染管线解耦
// =============================================================================
static GLuint compileShader(const char* vertSrc, const char* fragSrc) {
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

void PBREnvironment::InitCaptureResources() {
    // 1 创建FBO+深度RBO
    // 最大分辨率BRDF LUT用512×512
    constexpr uint32_t maxSize = 512;
    glGenFramebuffers(1, &s_captureFBO);
    glGenRenderbuffers(1, &s_captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, s_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, maxSize, maxSize);
    // 将RBO附加到FBO的深度附件
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, s_captureRBO);
    // 解绑FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // 解绑RBO
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // 2 创建Cubemap渲染用Cube VAO 立方体共6个面 每个面用2个三角形 每个三角形3个顶点 也就是每个面需要6个顶点 共36顶点
    // clang-format off
    float cubeVerts[] = {
        // -Y: bottom face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        // +Y: top face
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        // -Z: front face
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        // +Z: back face
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        // +X: right face
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        // -X: left face
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
    };
    // clang-format on

    {
        GLuint vbo;
        // 创建VAO
        glGenVertexArrays(1, &s_cubeVAO);
        // 创建VBO
        glGenBuffers(1, &vbo);
        // 绑定VAO插槽
        glBindVertexArray(s_cubeVAO);
        // 绑定VBO插槽
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // 顶点数据从内存传到显存
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts),  // 顶点数据多大
                     cubeVerts,  // 顶点数据在内存哪儿
                     GL_STATIC_DRAW);
        // vertex shader的layout=0的attribute变量 怎么从VBO取
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,  // 传给vertex shader的attribute=0位置
                              3,  // 每个顶点3个float
                              GL_FLOAT,  // 顶点pos用的是float类型
                              GL_FALSE,  // 不是归一化
                              0, nullptr);
        // 解绑VBO
        glBindVertexArray(0);
        // 解绑VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // 3 创建BRDF LUT渲染用Quad VAO 4顶点
    {
        // NDC空间的全屏四边形 (-1,-1), (1,-1), (-1,1), (1,1) 使用GL_TRIANGLE_STRIP模式绘制4个顶点=2个三角形
        float quadVerts[] = {-1, -1, 1, -1, -1, 1, 1, 1};
        GLuint vbo;
        glGenVertexArrays(1, &s_quadVAO);
        glGenBuffers(1, &vbo);
        glBindVertexArray(s_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        // 解绑VBO
        glBindVertexArray(0);
        // 解绑VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

/**
 * 释放GPU资源
 */
void PBREnvironment::ShutdownCaptureResources() {
    glDeleteFramebuffers(1, &s_captureFBO);
    glDeleteRenderbuffers(1, &s_captureRBO);
    glDeleteVertexArrays(1, &s_cubeVAO);
    glDeleteVertexArrays(1, &s_quadVAO);
    s_captureFBO = 0;
}

// =============================================================================
// PBREnvironment::BakeIrradiance() — 漫反射 IBL 辐照度图
//
// 原理: 对天空盒做半球积分卷积, 得到每个方向上的漫反射光照
//
//   输入: 原始 HDR Cubemap (天空盒)
//   输出: 低分辨率 Cubemap (默认 32×32), RGBA16F
//   方法: 对每个法线方向 N, 在半球上均匀采样天空盒,
//         用 cos(theta)*sin(theta) 作为权重做黎曼和积分
//   用途: PBR shader 中采样 u_IrradianceMap 得到环境漫反射分量
//   GLSL: irradiance = PI * Σ(envSample * cosθ * sinθ) / N_samples
// =============================================================================
X::Ref<TextureCube> PBREnvironment::BakeIrradiance(const X::Ref<TextureCube>& envMap, uint32_t size) {
    X_PROFILE_FUNCTION();

    const char* vert = R"(
#version 410 core
layout(location = 0) in vec3 a_Position;
out vec3 v_WorldPos;
uniform mat4 u_Projection;
uniform mat4 u_View;
void main() {
    v_WorldPos = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}
)";
    const char* frag = R"(
#version 410 core
in vec3 v_WorldPos;
out vec4 o_Color;
uniform samplerCube u_EnvironmentMap;
const float PI = 3.14159265359;
void main() {
    vec3 N = normalize(v_WorldPos);
    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));                        // 构建以 N 为 Z 轴的切线空间
    float sampleDelta = 0.025;
    int nrSamples = 0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance / float(nrSamples);        // 归一化 (黎曼和)
    o_Color = vec4(irradiance, 1.0);
}
)";
    GLuint prog = compileShader(vert, frag);

    // 创建输出纹理 (RGBA16F HDR)
    X::Ref<TextureCube> irradianceMap = TextureCube::Create(size, true);

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, CUBEMAP_CAPTURE_PROJECTION);
    glUniform1i(glGetUniformLocation(prog, "u_EnvironmentMap"), 0);
    glActiveTexture(GL_TEXTURE0);
    envMap->Bind(0);

    // 逐面渲染 cubemap 的 6 个面
    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    for (int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
        // 将当前 cubemap 面附着到 FBO 的颜色附件
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               irradianceMap->GetRendererID(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(s_cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // 恢复默认帧缓冲
    glDeleteProgram(prog);

    return irradianceMap;
}

// =============================================================================
// PBREnvironment::BakePrefilter() — 镜面反射 IBL 预过滤环境贴图
//
// 原理: 用 GGX 重要性采样对天空盒做预过滤, 每级 mip 对应不同的 roughness
//
//   输入: 原始 HDR Cubemap
//   输出: 带 mipmap 的 Cubemap (默认 128×128, 5级mip), RGBA16F
//
//   关键技术:
//     - Importance Sampling GGX: 用 Hammersley 低差异序列生成采样方向
//       使采样点集中在镜面反射波瓣 (roughness 越大波瓣越宽)
//     - mip level 选择: 根据采样方向的 PDF 计算等效固体角,
//       选择合适的 mip 级别来近似不同粗糙度下看到的模糊程度
//     - mip0 = roughness=0   (完全镜面, 从最大分辨率采样)
//       mip4 = roughness=1.0 (最粗糙, 从最小分辨率采样)
//
//   用途: PBR shader 中采样 u_PrefilterMap 得到环境镜面反射分量
// =============================================================================
X::Ref<TextureCube> PBREnvironment::BakePrefilter(const X::Ref<TextureCube>& envMap, uint32_t size, uint32_t maxMip) {
    X_PROFILE_FUNCTION();

    const char* vert = R"(
#version 410 core
layout(location = 0) in vec3 a_Position;
out vec3 v_WorldPos;
uniform mat4 u_Projection;
uniform mat4 u_View;
void main() {
    v_WorldPos = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}
)";
    const char* frag = R"(
#version 410 core
in vec3 v_WorldPos;
out vec4 o_Color;
uniform samplerCube u_EnvironmentMap;
uniform float u_Roughness;
const float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;             // ÷ 2^32
}
vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radicalInverse_VdC(i)); // 低差异序列 (均匀分布在 [0,1]²)
}
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;                              // 方位角
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);         // 极角 (GGX 分布逆变换采样)
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z); // 切线空间 → 世界空间
}
void main() {
    vec3 N = normalize(v_WorldPos);
    vec3 R = N;                                              // 对于 cubemap 渲染, R = N
    vec3 V = R;
    uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, u_Roughness);    // 按 GGX 分布生成半角向量
        vec3 L = normalize(2.0 * dot(V, H) * H - V);         // 反射得到光线方向
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            float D = DistributionGGX(N, H, u_Roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;  // GGX 概率密度函数
            float saTexel = 4.0 * PI / (6.0 * 512.0 * 512.0);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = u_Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); // 选择合适的 mip
            prefilteredColor += textureLod(u_EnvironmentMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;       // 加权平均
    o_Color = vec4(prefilteredColor, 1.0);
}
)";
    GLuint prog = compileShader(vert, frag);

    X::Ref<TextureCube> prefilterMap = TextureCube::Create(size, true);

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, CUBEMAP_CAPTURE_PROJECTION);
    glUniform1i(glGetUniformLocation(prog, "u_EnvironmentMap"), 0);
    glActiveTexture(GL_TEXTURE0);
    envMap->Bind(0);

    // 逐 mip 级别渲染 (roughness 递增)
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    for (uint32_t mip = 0; mip < maxMip; ++mip) {
        uint32_t mipSize = size >> mip;  // 每级 mip 分辨率减半
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);  // 调整深度缓冲大小
        glViewport(0, 0, mipSize, mipSize);

        float roughness = (float)mip / (float)(maxMip - 1);  // mip0=roughness0, mip4=roughness1.0
        glUniform1f(glGetUniformLocation(prog, "u_Roughness"), roughness);

        // 逐面渲染 6 个 cubemap 面
        for (int i = 0; i < 6; ++i) {
            glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   prefilterMap->GetRendererID(), mip);  // 渲染到指定 mip 级别
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(s_cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteProgram(prog);

    return prefilterMap;
}

// =============================================================================
// PBREnvironment::BakeBRDFLUT() — BRDF 积分查找表
//
// 原理: 将 Cook-Torrance BRDF 中与材质无关的部分预计算成 2D 查找表
//   F0 (Fresnel 反射率) 由运行时材质 metallic/albedo 决定, 运行时计算
//   BRDF LUT 只存储与 NdotV 和 roughness 相关的积分结果
//
//   输入: 无 (纯数学积分)
//   输出: 512×512 RG16F 2D 纹理
//         R 通道 = BRDF 积分 scale (Fresnel 项的系数)
//         G 通道 = BRDF 积分 bias  (Fresnel 项的偏移)
//   X 轴 = NdotV (0→1, 视线与法线夹角余弦)
//   Y 轴 = roughness (0→1, 粗糙度)
//
//   Split-Sum Approximation:
//     ∫(fresnel * brdf) dωi ≈ F0 * scale(roughness, NdotV) + bias(roughness, NdotV)
//
//   用途: PBR shader 中采样 u_BRDFLUT 做镜面 IBL 的快速近似
// =============================================================================
uint32_t PBREnvironment::BakeBRDFLUT(uint32_t size) {
    X_PROFILE_FUNCTION();

    const char* vert = R"(
#version 410 core
layout(location = 0) in vec2 a_Position;
out vec2 v_TexCoord;
void main() {
    v_TexCoord = a_Position * 0.5 + 0.5;                  // NDC [-1,1] → UV [0,1]
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";
    const char* frag = R"(
#version 410 core
in vec2 v_TexCoord;
out vec2 o_Color;                                           // RG16F 输出: (scale, bias)
const float PI = 3.14159265359;
float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}
float GeometrySchlickGGX(float NdotV, float roughness) {
    float k = (roughness * roughness) / 2.0;                // 直接光照的 k 值
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}
vec2 IntegrateBRDF(float NdotV, float roughness) {
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);   // 构建视线向量 (XZ 平面)
    vec3 N = vec3(0.0, 0.0, 1.0);                            // 法线始终朝 +Z
    float A = 0.0, B = 0.0;                                  // A=scale, B=bias
    uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);         // 反射得到光线方向
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0) {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);     // 可见项 (visibility term)
            float Fc = pow(1.0 - VdotH, 5.0);                // Fresnel-Schlick 近似
            A += (1.0 - Fc) * G_Vis;                         // scale 累积 (F0 系数)
            B += Fc * G_Vis;                                  // bias 累积 (偏移项)
        }
    }
    return vec2(A, B) / float(SAMPLE_COUNT);                 // Monte Carlo 平均
}
void main() {
    vec2 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y);
    o_Color = integratedBRDF;
}
)";
    GLuint prog = compileShader(vert, frag);

    // 创建输出纹理 (RG16F, 两个通道存储 scale 和 bias)
    GLuint brdfLUT;
    glGenTextures(1, &brdfLUT);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 渲染全屏四边形一次 (单 pass)
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);
    glViewport(0, 0, size, size);
    glUseProgram(prog);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // 绘制全屏四边形
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteProgram(prog);

    return brdfLUT;
}
