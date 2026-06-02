//
// Created by rui ding on 2026/6/1.
//

#include "pbr_env.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

// Capture matrices (same as texture_cube.cpp)
static constexpr float CUBEMAP_CAPTURE_PROJECTION[16] = {
    1.0f, 0.0f,  0.0f,  0.0f,  //
    0.0f, -1.0f, 0.0f,  0.0f,  //
    0.0f, 0.0f,  -0.5f, 0.0f,  //
    0.0f, 0.0f,  0.0f,  1.0f   //
};

static const glm::mat4 CAPTURE_VIEWS[6] = {
    glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
    glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
    glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)),
};

GLuint PBREnvironment::s_captureFBO = 0;
GLuint PBREnvironment::s_captureRBO = 0;
GLuint PBREnvironment::s_cubeVAO = 0;
GLuint PBREnvironment::s_quadVAO = 0;

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
    constexpr uint32_t maxSize = 512;
    glGenFramebuffers(1, &s_captureFBO);
    glGenRenderbuffers(1, &s_captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, s_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, maxSize, maxSize);

    float cubeVerts[] = {
        -1, -1, -1, -1, -1, 1,  1,  -1, 1,  1,  -1, -1,  //
        -1, 1,  -1, -1, 1,  1,  1,  1,  1,  1,  1,  -1,  //
        -1, -1, -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1,  //
        -1, -1, 1,  -1, 1,  1,  1,  1,  1,  1,  -1, 1,   //
        1,  -1, -1, 1,  1,  -1, 1,  1,  1,  1,  -1, 1,   //
        -1, -1, -1, -1, 1,  -1, -1, 1,  1,  -1, -1, 1    //
    };
    {
        GLuint vbo;
        glGenVertexArrays(1, &s_cubeVAO);
        glGenBuffers(1, &vbo);
        glBindVertexArray(s_cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    // Quad VAO
    {
        float quadVerts[] = {-1, -1, 1, -1, -1, 1, 1, 1};
        GLuint vbo;
        glGenVertexArrays(1, &s_quadVAO);
        glGenBuffers(1, &vbo);
        glBindVertexArray(s_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
}

void PBREnvironment::ShutdownCaptureResources() {
    glDeleteFramebuffers(1, &s_captureFBO);
    glDeleteRenderbuffers(1, &s_captureRBO);
    glDeleteVertexArrays(1, &s_cubeVAO);
    glDeleteVertexArrays(1, &s_quadVAO);
    s_captureFBO = 0;
}

// ---- Irradiance map (diffuse IBL) ----

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
    up = normalize(cross(N, right));
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
    irradiance = PI * irradiance / float(nrSamples);
    o_Color = vec4(irradiance, 1.0);
}
)";
    GLuint prog = compileShader(vert, frag);

    X::Ref<TextureCube> irradianceMap = TextureCube::Create(size, true);

    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, CUBEMAP_CAPTURE_PROJECTION);
    glUniform1i(glGetUniformLocation(prog, "u_EnvironmentMap"), 0);
    glActiveTexture(GL_TEXTURE0);
    envMap->Bind(0);

    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    for (int i = 0; i < 6; ++i) {
        glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               irradianceMap->GetRendererID(), 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(s_cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteProgram(prog);

    return irradianceMap;
}

// ---- Prefiltered environment map (specular IBL) ----

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
void main() {
    vec3 N = normalize(v_WorldPos);
    vec3 R = N;
    vec3 V = R;
    uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, u_Roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            float D = DistributionGGX(N, H, u_Roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;
            float saTexel = 4.0 * PI / (6.0 * 512.0 * 512.0);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = u_Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            prefilteredColor += textureLod(u_EnvironmentMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
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

    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    for (uint32_t mip = 0; mip < maxMip; ++mip) {
        uint32_t mipSize = size >> mip;
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
        glViewport(0, 0, mipSize, mipSize);

        float roughness = (float)mip / (float)(maxMip - 1);
        glUniform1f(glGetUniformLocation(prog, "u_Roughness"), roughness);

        for (int i = 0; i < 6; ++i) {
            glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, glm::value_ptr(CAPTURE_VIEWS[i]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   prefilterMap->GetRendererID(), mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(s_cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteProgram(prog);

    return prefilterMap;
}

// ---- BRDF LUT ----

uint32_t PBREnvironment::BakeBRDFLUT(uint32_t size) {
    X_PROFILE_FUNCTION();

    const char* vert = R"(
#version 410 core
layout(location = 0) in vec2 a_Position;
out vec2 v_TexCoord;
void main() {
    v_TexCoord = a_Position * 0.5 + 0.5;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";
    const char* frag = R"(
#version 410 core
in vec2 v_TexCoord;
out vec2 o_Color;
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
    float k = (roughness * roughness) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}
vec2 IntegrateBRDF(float NdotV, float roughness) {
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);
    float A = 0.0, B = 0.0;
    uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0) {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return vec2(A, B) / float(SAMPLE_COUNT);
}
void main() {
    vec2 integratedBRDF = IntegrateBRDF(v_TexCoord.x, v_TexCoord.y);
    o_Color = integratedBRDF;
}
)";
    GLuint prog = compileShader(vert, frag);

    GLuint brdfLUT;
    glGenTextures(1, &brdfLUT);
    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, size, size, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, s_captureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);
    glViewport(0, 0, size, size);
    glUseProgram(prog);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteProgram(prog);

    return brdfLUT;
}
