//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/3d/renderer_3D.h"

#include "advanced_effects.h"
#include "pbr_env.h"
#include "renderer_3D_internal.h"
#include "x/renderer/buffer/uniform_buffer.h"
#include "x/renderer/camera/camera.h"
#include "x/renderer/camera/editor_camera.h"
#include "x/renderer/render_command.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

/**
 * 全局渲染状态 单例 整个Renderer3D只有一份
 * 渲染架构
 * BeginScene → DrawMesh*N → EndScene → Flush()
 *                              ↑
 *                            每帧累积MeshDrawCommand
 *                            按Material分组入Buckets
 *                            Flush时统一提交GPU绘制
 */
static Renderer3DData s_data;

void Renderer3D::Init() {
    X_PROFILE_FUNCTION();
    // 1 PBR环境捕获工具 天空盒HDR到Cube map转换+HDR滤波器
    PBREnvironment::InitCaptureResources();
    // 2 后处理全屏四边形 只有一个quad的VAO
    FullscreenQuad::Init();
    // 3 创建Shader对象
    s_data.DefaultShader = Shader::Create("asset/shader/Renderer3D_Phong.glsl");
    s_data.PBRShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");
    s_data.SkyboxShader = Shader::Create("asset/shader/Skybox.glsl");

    /**
     * 4 显式绑定UBO Block到binding slot 为什么需要这一步
     *   - 1 虽然GLSL源码中已声明layout(std140, binding=N)
     *   - 2 但某些OpenGL驱动 尤其是SPIRV路径可能不认GLSL中的binding声明
     *   - 3 需要用glUniformBlockBinding运行时再次显式绑定
     * 每个UBO对应的binding点
     */
    static constexpr struct {
        const char* Name;  // UBO的名称
        uint32_t Binding;  // UBO在shader里面对应的binding
    } s_UBODefs[] = {
        {"Camera", UBOBinding::Camera},
        {"Model", UBOBinding::Model},
        {"LightBlock", UBOBinding::Light},
        {"PBRSettings", UBOBinding::PBRSettings},
        {"LightSpace", UBOBinding::LightSpace},
        {"CSMData", UBOBinding::CSMData},
    };

    // 手动绑定UBO的block跟binding
    auto setupBlockBindings = [](const Ref<Shader>& shader) {
        // OpenGL维护的shader object
        GLuint prog = shader->GetRendererID();
        for (auto& def : s_UBODefs) {
            // 从shader里面拿到UBO对应的block
            GLuint idx = glGetUniformBlockIndex(prog, def.Name);
            if (idx != GL_INVALID_INDEX) {
                // 把UBO的block跟binding手动绑起来
                glUniformBlockBinding(prog, idx, def.Binding);
            }
        }
    };
    // 冯氏shader
    setupBlockBindings(s_data.DefaultShader);
    setupBlockBindings(s_data.PBRShader);
    setupBlockBindings(s_data.SkyboxShader);
    // 5 白色纹理 没有贴图时的默认材质
    s_data.WhiteTexture = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;  // RGBA 全白
    s_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
    /**
     * 6 创建UBO缓冲区
     * glGenBuffers+glBindBuffer(GL_UNIFORM_BUFFER)+glBufferData分配显存
     * 然后用glBindBufferBase绑定到指定的binding slot
     */
    // Camera UBO (binding=0) 所有 shader共享
    s_data.CameraUBO = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), UBOBinding::Camera);
    // Model UBO (binding=1) 每个物体不同 Flush 时每物体更新
    s_data.ModelUBO = UniformBuffer::Create(sizeof(glm::mat4), UBOBinding::Model);
    // 7 默认光源参数
    LightGroupData defaultLight{};
    defaultLight.Ambient = glm::vec3(0.05f);
    defaultLight.LightCount = 1;
    defaultLight.Lights[0].PositionAndRange = glm::vec4(-0.5f, -1.0f, -0.3f, 0.0f);
    defaultLight.Lights[0].ColorAndIntensity = glm::vec4(1.0f, 0.95f, 0.9f, 1.0f);
    defaultLight.Lights[0].Type = kDirectional;
    s_data.LightGroups[0] = defaultLight;
    s_data.LightUBO = UniformBuffer::Create(sizeof(LightGroupData), UBOBinding::Light);
    // 8 PBR 默认参数
    s_data.PBRBuffer.CameraPosition = glm::vec3(0.0f);
    s_data.PBRBuffer.Exposure = 1.0f;
    s_data.PBRUBO = UniformBuffer::Create(sizeof(PBRSettingsData), UBOBinding::PBRSettings);
    /**
     * 9 默认Cubemap 黑色
     * PBR shader需要IrradianceMap/PrefilterMap采样器
     * 在没有加载天空盒时 绑定这个黑色 cubemap避免采样未定义的纹理单元
     */
    {
        Ref<TextureCube> defaultCube = TextureCube::Create(1, true);
        s_data.IrradianceMap = defaultCube;
        s_data.PrefilterMap = defaultCube;
    }
    /**
     * 10 默认BRDF LUT 白色1×1 RG16F
     * RG16F
     *   - R=1.0 对应BRDF积分scale
     *   - G=0.0(对应 bias)
     * 白色表示无镜面IBL贡献
     */
    {
        float brdfData[] = {1.0f, 0.0f, 1.0f, 0.0f};  // RG16F: 2 个像素 (因为每个像素2通道)
        glGenTextures(1, &s_data.BRDFLUTTexture);
        glBindTexture(GL_TEXTURE_2D, s_data.BRDFLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1, 1, 0, GL_RG, GL_FLOAT, brdfData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    // 11 默认Shadow Map 白色深度=无阴影
    {
        float whiteDepth = 1.0f;  // 1.0=最远深度=不被遮挡
        glGenTextures(1, &s_data.DefaultShadowMap);
        glBindTexture(GL_TEXTURE_2D, s_data.DefaultShadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &whiteDepth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border[] = {1.0f, 1.0f, 1.0f, 1.0f};  // border 颜色为白 (采样到边界外=无阴影)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    }
}

/**
 * 释放所有GPU资源
 */
void Renderer3D::Shutdown() {
    X_PROFILE_FUNCTION();
    s_data.DefaultShader.reset();
    s_data.PBRShader.reset();
    s_data.SkyboxShader.reset();
    s_data.WhiteTexture.reset();
    s_data.CameraUBO.reset();
    s_data.ModelUBO.reset();
    s_data.LightUBO.reset();
    s_data.PBRUBO.reset();
    s_data.EnvironmentMap.reset();
    s_data.IrradianceMap.reset();
    s_data.PrefilterMap.reset();
    if (s_data.BRDFLUTTexture) {
        glDeleteTextures(1, &s_data.BRDFLUTTexture);
        s_data.BRDFLUTTexture = 0;
    }
    if (s_data.DefaultShadowMap) {
        glDeleteTextures(1, &s_data.DefaultShadowMap);
        s_data.DefaultShadowMap = 0;
    }
    s_data.Buckets.clear();
    PBREnvironment::ShutdownCaptureResources();
}

/**
 * 视口大小变化时调用
 * @param width 窗口要变成多宽
 * @param height 窗口要变成多高
 */
void Renderer3D::SetViewport(uint32_t width, uint32_t height) {
    RenderCommand::SetViewport(0, 0, width, height);
}

/**
 * 每帧开始时调用
 *   1 清空上一帧的Buckets里面的累积
 *   2 上传Camera/Light/PBRSettings这些UBO数据到GPU
 * @param camera
 * @param viewMatrix 外部的view矩阵
 */
void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& viewMatrix) {
    X_PROFILE_FUNCTION();
    s_data.CurrentViewMatrix = viewMatrix;
    s_data.CurrentProjectionMatrix = camera.get_projection();
    s_data.CameraUBO->Bind();
    s_data.CameraBuffer.ViewProjection = s_data.CurrentProjectionMatrix * viewMatrix;
    s_data.CameraUBO->SetData(&s_data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    s_data.LightUBO->SetData(&s_data.LightGroups[0], sizeof(LightGroupData));
    s_data.PBRBuffer.CameraPosition = glm::vec3(glm::inverse(viewMatrix)[3]);
    s_data.PBRUBO->SetData(&s_data.PBRBuffer, sizeof(PBRSettingsData));
    s_data.Buckets.clear();
    s_data.Stats = {};
}

void Renderer3D::BeginScene(const EditorCamera& camera) {
    X_PROFILE_FUNCTION();
    s_data.CurrentViewMatrix = camera.get_viewMatrix();
    s_data.CurrentProjectionMatrix = camera.get_projection();
    s_data.CameraUBO->Bind();
    s_data.CameraBuffer.ViewProjection = camera.GetViewProjection();
    s_data.CameraUBO->SetData(&s_data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    s_data.LightUBO->SetData(&s_data.LightGroups[0], sizeof(LightGroupData));
    s_data.PBRBuffer.CameraPosition = camera.get_position();
    s_data.PBRUBO->SetData(&s_data.PBRBuffer, sizeof(PBRSettingsData));
    s_data.Buckets.clear();
    s_data.Stats = {};
}

void Renderer3D::EndScene() {
    X_PROFILE_FUNCTION();
    Flush();
}

/**
 *
 * UBOS绑定
 *   Camera      binding=0  ← 帧级 BeginScene已更新
 *   Model       binding=1  ← 物体级 每物体更新
 *   Light       binding=2  ← 帧级 BeginScene已更新
 *   PBRSettings binding=3  ← 帧级 BeginScene已更新
 *
 * 纹理单元
 *   GL_TEXTURE0  — 用户纹理 Material::Bind内部绑定
 *   ...
 *   GL_TEXTURE4  — IrradianceMap   IBL漫反射
 *   GL_TEXTURE5  — PrefilterMap    IBL镜面反射
 *   GL_TEXTURE6  — BRDF LUT        IBL BRDF积分
 *   GL_TEXTURE7  — ShadowMap0      CSM级联0
 *   GL_TEXTURE8  — ShadowMap1      CSM级联1
 *   GL_TEXTURE9  — ShadowMap2      CSM级联2
 *   GL_TEXTURE10 — ShadowMap3      CSM级联3
 */
void Renderer3D::Flush() {
    if (s_data.Buckets.empty()) return;

    // 绑定IBL 纹理到固定纹理单元
    if (s_data.IrradianceMap) {
        s_data.IrradianceMap->Bind(4);
    }
    if (s_data.PrefilterMap) {
        s_data.PrefilterMap->Bind(5);
    }
    if (s_data.BRDFLUTTexture) {
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, s_data.BRDFLUTTexture);
    }
    // 绑定默认Shadow Map 4个级联
    for (int i = 0; i < 4; ++i) {
        glActiveTexture(GL_TEXTURE7 + i);
        glBindTexture(GL_TEXTURE_2D, s_data.DefaultShadowMap);
    }

    // 按(lightGroup, shader)排序Buckets 减少状态切换
    std::sort(s_data.Buckets.begin(), s_data.Buckets.end(), [](const MaterialBucket& a, const MaterialBucket& b) {
        uint32_t groupA = a.MaterialAsset->GetLightGroup();
        uint32_t groupB = b.MaterialAsset->GetLightGroup();
        if (groupA != groupB) return groupA < groupB;
        return a.MaterialAsset->GetShader().get() < b.MaterialAsset->GetShader().get();
    });

    uint32_t currentLightGroup = UINT32_MAX;

    // 遍历每个Material Bucket
    for (auto& bucket : s_data.Buckets) {
        uint32_t bucketGroup = bucket.MaterialAsset->GetLightGroup();
        if (bucketGroup != currentLightGroup) {
            currentLightGroup = bucketGroup;
            auto it = s_data.LightGroups.find(bucketGroup);
            if (it != s_data.LightGroups.end()) {
                s_data.LightUBO->SetData(&it->second, sizeof(LightGroupData));
            }
        }
        // 激活shader 绑定纹理
        bucket.MaterialAsset->Bind();

        auto& shader = bucket.MaterialAsset->GetShader();
        if (shader->HasUniform("u_IrradianceMap")) shader->SetInt("u_IrradianceMap", 4);
        if (shader->HasUniform("u_PrefilterMap")) shader->SetInt("u_PrefilterMap", 5);
        if (shader->HasUniform("u_BRDFLUT")) shader->SetInt("u_BRDFLUT", 6);
        if (shader->HasUniform("u_ShadowMap0")) shader->SetInt("u_ShadowMap0", 7);
        if (shader->HasUniform("u_ShadowMap1")) shader->SetInt("u_ShadowMap1", 8);
        if (shader->HasUniform("u_ShadowMap2")) shader->SetInt("u_ShadowMap2", 9);
        if (shader->HasUniform("u_ShadowMap3")) shader->SetInt("u_ShadowMap3", 10);

        // 遍历该Material下的每个绘制命令
        for (auto& cmd : bucket.Commands) {
            // 更新UBO 模型矩阵 负责把本地坐标转换成世界坐标
            s_data.ModelUBO->SetData(glm::value_ptr(cmd.Transform), sizeof(glm::mat4));
            // 设置EntityID 用于帧缓冲读回实现鼠标拾取
            shader->SetInt("u_EntityID", cmd.EntityID);

            // 提交GPU绘制
            auto& vao = cmd.MeshAsset->GetVertexArray();
            RenderCommand::DrawIndexed(vao, cmd.MeshAsset->GetIndexCount());

            s_data.Stats.DrawCalls++;
        }
        s_data.Stats.MeshCount += static_cast<uint32_t>(bucket.Commands.size());
    }
}

void Renderer3D::DrawMesh(const Ref<Mesh>& mesh, const Ref<Material>& material, const glm::mat4& transform,
                          int32_t entityID) {
    X_PROFILE_FUNCTION();
    if (!mesh || !material) {
        return;
    }
    // 看看这个材质是不是已经有其他的绘制指令了 可能提交过来的多个mesh其实画的都是一种材质
    for (auto& bucket : s_data.Buckets) {
        if (*bucket.MaterialAsset == *material) {
            bucket.Commands.push_back({mesh, transform, entityID});
            return;
        }
    }
    // 有个新的材质要绘制
    MaterialBucket newBucket{};
    newBucket.MaterialAsset = material;
    newBucket.Commands.push_back({mesh, transform, entityID});
    s_data.Buckets.push_back(std::move(newBucket));
}

void Renderer3D::DrawSkybox() {
    if (!s_data.EnvironmentMap || !s_data.SkyboxShader) return;

    // 创建天空盒VAO/VBO static修饰只初始化一次
    static GLuint skyboxVAO = 0, skyboxVBO = 0;
    if (!skyboxVAO) {
        // 天空盒 正方体 6个面 每个面2个三角形 共36个顶点
        // clang-format off
        float verts[] = {
            // bottom (y = -1)
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
            // top (y = 1)
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
            // back (z = -1)
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            // front (z = 1)
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            // right (x = 1)
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            // left (x = -1)
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f
        };
        // clang-format on
        // 创建VAO
        glGenVertexArrays(1, &skyboxVAO);
        // 创建VBO
        glGenBuffers(1, &skyboxVBO);
        // 绑定VAO插槽
        glBindVertexArray(skyboxVAO);
        // 绑定VBO插槽
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        // 把顶点数据从内存复制到显存
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        // 告诉GPU怎么解读attriburte0的顶点数据
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3,  // 每个顶点3个float
                              GL_FLOAT,  // 顶点数据是float类型
                              GL_FALSE,  // 不是归一化
                              0,  // 偏移0
                              nullptr);
    }

    // LEQUAL 天空盒永远在最远深度 也就是深度=1.0处绘制
    glDepthFunc(GL_LEQUAL);
    s_data.SkyboxShader->Bind();

    // 天空盒去除视图矩阵的位移 使其始终以摄像机为中心
    glm::mat4 skyboxView = glm::mat4(glm::mat3(s_data.CurrentViewMatrix));
    glm::mat4 skyboxVP = s_data.CurrentProjectionMatrix * skyboxView;
    s_data.SkyboxShader->SetMat4("u_ViewProjection", skyboxVP);
    s_data.SkyboxShader->SetInt("u_Skybox", 0);
    s_data.SkyboxShader->SetFloat("u_Exposure", s_data.PBRBuffer.Exposure);

    s_data.EnvironmentMap->Bind(0);
    // 绑定VAO插槽
    glBindVertexArray(skyboxVAO);
    // 用36个顶点画天空盒
    glDrawArrays(GL_TRIANGLES, 0,  // 从0号顶点开始画
                 36  // 用36个顶点画
    );
    // 恢复默认深度测试
    glDepthFunc(GL_LESS);
}

void Renderer3D::SetEnvironmentMap(const Ref<TextureCube>& envMap) {
    s_data.EnvironmentMap = envMap;
    if (envMap) {
        // 从天空盒HDR Cubemap预计算IBL数据
        s_data.IrradianceMap = PBREnvironment::BakeIrradiance(envMap);  // 漫反射IBL
        s_data.PrefilterMap = PBREnvironment::BakePrefilter(envMap);  // 镜面反射IBL 多级roughness
        s_data.BRDFLUTTexture = PBREnvironment::BakeBRDFLUT();  // BRDF积分查找表
    }
}

void Renderer3D::SetEnvironmentMaps(const Ref<TextureCube>& envMap, const Ref<TextureCube>& irradianceMap,
                                    const Ref<TextureCube>& prefilterMap, uint32_t brdfLUTTexture) {
    s_data.EnvironmentMap = envMap;
    s_data.IrradianceMap = irradianceMap;
    s_data.PrefilterMap = prefilterMap;
    s_data.BRDFLUTTexture = brdfLUTTexture;
}

void Renderer3D::SetExposure(float exposure) {
    s_data.PBRBuffer.Exposure = exposure;
}

const Ref<TextureCube>& Renderer3D::GetEnvironmentMap() {
    return s_data.EnvironmentMap;
}

void Renderer3D::SetLightAmbient(const glm::vec3& ambient, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Ambient = ambient;
}

void Renderer3D::SetLightCount(uint32_t count, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].LightCount = static_cast<int>(std::min(count, MAX_GPU_LIGHTS));
}

void Renderer3D::SetDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity,
                                     uint32_t lightGroupId, uint32_t lightIndex) {
    auto& light = s_data.LightGroups[lightGroupId].Lights[lightIndex];
    light.PositionAndRange = glm::vec4(direction, 0.0f);
    light.ColorAndIntensity = glm::vec4(color, intensity);
    light.Type = kDirectional;
}

void Renderer3D::SetPointLight(const glm::vec3& position, const glm::vec3& color, float range, float intensity,
                               uint32_t lightGroupId, uint32_t lightIndex) {
    auto& light = s_data.LightGroups[lightGroupId].Lights[lightIndex];
    light.PositionAndRange = glm::vec4(position, range);
    light.ColorAndIntensity = glm::vec4(color, intensity);
    light.Type = kPoint;
}

void Renderer3D::SetLightDirection(const glm::vec3& direction, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.x = direction.x;
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.y = direction.y;
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.z = direction.z;
}

void Renderer3D::SetLightColor(const glm::vec3& color, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.x = color.x;
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.y = color.y;
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.z = color.z;
}

void Renderer3D::SetPointLightPosition(const glm::vec3& position, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.x = position.x;
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.y = position.y;
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.z = position.z;
    s_data.LightGroups[lightGroupId].Lights[0].Type = kPoint;
}

void Renderer3D::SetPointLightColor(const glm::vec3& color, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.x = color.x;
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.y = color.y;
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.z = color.z;
}

void Renderer3D::SetPointLightRange(float range, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].PositionAndRange.w = range;
}

void Renderer3D::SetPointLightIntensity(float intensity, uint32_t lightGroupId) {
    s_data.LightGroups[lightGroupId].Lights[0].ColorAndIntensity.w = intensity;
}

Renderer3D::Statistics Renderer3D::GetStats() {
    return s_data.Stats;
}

void Renderer3D::ResetStats() {
    s_data.Stats = {};
}
