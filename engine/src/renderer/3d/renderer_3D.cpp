//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/renderer_3D.h"

#include "advanced_effects.h"
#include "pbr_env.h"
#include "renderer_3D_internal.h"
#include "x/renderer/camera.h"
#include "x/renderer/editor_camera.h"
#include "x/renderer/render_command.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/uniform_buffer.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

static Renderer3DData s_data;

void Renderer3D::Init() {
    X_PROFILE_FUNCTION();

    PBREnvironment::InitCaptureResources();
    FullscreenQuad::Init();

    s_data.DefaultShader = Shader::Create("asset/shader/Renderer3D_Phong.glsl");
    s_data.PBRShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");
    s_data.SkyboxShader = Shader::Create("asset/shader/Skybox.glsl");

    // Set UBO block bindings for all shaders (needed on platforms without layout(binding=N))
    auto setupBlockBindings = [](const X::Ref<Shader>& shader) {
        GLuint prog = shader->GetRendererID();
        GLuint idx = glGetUniformBlockIndex(prog, "Camera");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 0);
        idx = glGetUniformBlockIndex(prog, "Model");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 1);
        idx = glGetUniformBlockIndex(prog, "Light");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 2);
        idx = glGetUniformBlockIndex(prog, "PBRSettings");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 3);
        idx = glGetUniformBlockIndex(prog, "LightSpace");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 4);
        idx = glGetUniformBlockIndex(prog, "CSMData");
        if (idx != GL_INVALID_INDEX) glUniformBlockBinding(prog, idx, 5);
    };
    setupBlockBindings(s_data.DefaultShader);
    setupBlockBindings(s_data.PBRShader);
    setupBlockBindings(s_data.SkyboxShader);

    s_data.WhiteTexture = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    s_data.CameraUBO = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
    s_data.ModelUBO = UniformBuffer::Create(sizeof(glm::mat4), 1);

    LightData defaultLight;
    defaultLight.Direction = glm::vec3(-0.5f, -1.0f, -0.3f);
    defaultLight.Ambient = glm::vec3(0.05f);
    defaultLight.Diffuse = glm::vec3(1.0f);
    defaultLight.Specular = glm::vec3(1.0f);
    s_data.LightBuffer = defaultLight;
    s_data.LightUBO = UniformBuffer::Create(sizeof(LightData), 2);

    s_data.PBRBuffer.CameraPosition = glm::vec3(0.0f);
    s_data.PBRBuffer.Exposure = 1.0f;
    s_data.PBRUBO = UniformBuffer::Create(sizeof(PBRSettingsData), 3);

    // Default cubemap (black) so PBR shader doesn't sample from undefined texture units
    {
        X::Ref<TextureCube> defaultCube = TextureCube::Create(1, true);
        s_data.IrradianceMap = defaultCube;
        s_data.PrefilterMap = defaultCube;
    }

    // Default BRDF LUT (white 1x1 RG16F — no specular IBL contribution)
    {
        float brdfData[] = {1.0f, 0.0f, 1.0f, 0.0f};
        glGenTextures(1, &s_data.BRDFLUTTexture);
        glBindTexture(GL_TEXTURE_2D, s_data.BRDFLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 1, 1, 0, GL_RG, GL_FLOAT, brdfData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Default shadow map (white = no shadow)
    {
        float whiteDepth = 1.0f;
        glGenTextures(1, &s_data.DefaultShadowMap);
        glBindTexture(GL_TEXTURE_2D, s_data.DefaultShadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &whiteDepth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    }
}

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

void Renderer3D::SetViewport(uint32_t width, uint32_t height) {
    RenderCommand::SetViewport(0, 0, width, height);
}

void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& viewMatrix) {
    X_PROFILE_FUNCTION();
    s_data.CameraUBO->Bind();
    s_data.CameraBuffer.ViewProjection = camera.get_projection() * viewMatrix;
    s_data.CameraUBO->SetData(&s_data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    s_data.LightUBO->SetData(&s_data.LightBuffer, sizeof(LightData));
    s_data.PBRBuffer.CameraPosition = glm::vec3(glm::inverse(viewMatrix)[3]);
    s_data.PBRUBO->SetData(&s_data.PBRBuffer, sizeof(PBRSettingsData));
    s_data.Buckets.clear();
    s_data.Stats = {};
}

void Renderer3D::BeginScene(const EditorCamera& camera) {
    X_PROFILE_FUNCTION();
    s_data.CameraUBO->Bind();
    s_data.CameraBuffer.ViewProjection = camera.GetViewProjection();
    s_data.CameraUBO->SetData(&s_data.CameraBuffer, sizeof(Renderer3DData::CameraData));
    s_data.LightUBO->SetData(&s_data.LightBuffer, sizeof(LightData));
    s_data.PBRBuffer.CameraPosition = camera.get_position();
    s_data.PBRUBO->SetData(&s_data.PBRBuffer, sizeof(PBRSettingsData));
    s_data.Buckets.clear();
    s_data.Stats = {};
}

void Renderer3D::EndScene() {
    X_PROFILE_FUNCTION();
    Flush();
}

void Renderer3D::Flush() {
    if (s_data.Buckets.empty()) return;

    // Bind IBL textures for PBR shader
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
    // Bind default shadow maps (no shadow)
    for (int i = 0; i < 4; ++i) {
        glActiveTexture(GL_TEXTURE7 + i);
        glBindTexture(GL_TEXTURE_2D, s_data.DefaultShadowMap);
    }

    std::sort(s_data.Buckets.begin(), s_data.Buckets.end(), [](const MaterialBucket& a, const MaterialBucket& b) {
        return a.Material->GetShader().get() < b.Material->GetShader().get();
    });

    for (auto& bucket : s_data.Buckets) {
        bucket.Material->Bind();
        for (auto& cmd : bucket.Commands) {
            s_data.ModelUBO->SetData(glm::value_ptr(cmd.Transform), sizeof(glm::mat4));
            bucket.Material->GetShader()->SetInt("u_EntityID", cmd.EntityID);

            // Set IBL sampler uniforms
            auto& shader = bucket.Material->GetShader();
            shader->SetInt("u_IrradianceMap", 4);
            shader->SetInt("u_PrefilterMap", 5);
            shader->SetInt("u_BRDFLUT", 6);
            shader->SetInt("u_ShadowMap0", 7);
            shader->SetInt("u_ShadowMap1", 8);
            shader->SetInt("u_ShadowMap2", 9);
            shader->SetInt("u_ShadowMap3", 10);

            auto& vao = cmd.Mesh->GetVertexArray();
            RenderCommand::DrawIndexed(vao, cmd.Mesh->GetIndexCount());

            s_data.Stats.DrawCalls++;
        }
        s_data.Stats.MeshCount += static_cast<uint32_t>(bucket.Commands.size());
    }
}

void Renderer3D::DrawMesh(const X::Ref<Mesh>& mesh, const X::Ref<Material>& material, const glm::mat4& transform,
                          int32_t entityID) {
    X_PROFILE_FUNCTION();
    if (!mesh || !material) return;

    for (auto& bucket : s_data.Buckets) {
        if (*bucket.Material == *material) {
            bucket.Commands.push_back({mesh, transform, entityID});
            return;
        }
    }

    MaterialBucket newBucket;
    newBucket.Material = material;
    newBucket.Commands.push_back({mesh, transform, entityID});
    s_data.Buckets.push_back(std::move(newBucket));
}

void Renderer3D::DrawSkybox() {
    if (!s_data.EnvironmentMap || !s_data.SkyboxShader) return;

    // Use the same cube VAO from PBREnvironment
    static GLuint skyboxVAO = 0, skyboxVBO = 0;
    if (!skyboxVAO) {
        float verts[] = {-1, -1, -1, -1, -1, 1,  1, -1, 1,  1, -1, -1, -1, 1,  -1, -1, 1, 1,  1,  1, 1, 1,  1,  -1,
                         -1, -1, -1, -1, 1,  -1, 1, 1,  -1, 1, -1, -1, -1, -1, 1,  -1, 1, 1,  1,  1, 1, 1,  -1, 1,
                         1,  -1, -1, 1,  1,  -1, 1, 1,  1,  1, -1, 1,  -1, -1, -1, -1, 1, -1, -1, 1, 1, -1, -1, 1};
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glDepthFunc(GL_LEQUAL);
    s_data.SkyboxShader->Bind();

    glm::mat4 skyboxVP = s_data.CameraBuffer.ViewProjection;
    s_data.SkyboxShader->SetMat4("u_ViewProjection", skyboxVP);
    s_data.SkyboxShader->SetInt("u_Skybox", 0);
    s_data.SkyboxShader->SetFloat("u_Exposure", s_data.PBRBuffer.Exposure);

    s_data.EnvironmentMap->Bind(0);
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

// ---- PBR Environment ----

void Renderer3D::SetEnvironmentMap(const X::Ref<TextureCube>& envMap) {
    s_data.EnvironmentMap = envMap;
    if (envMap) {
        s_data.IrradianceMap = PBREnvironment::BakeIrradiance(envMap);
        s_data.PrefilterMap = PBREnvironment::BakePrefilter(envMap);
        s_data.BRDFLUTTexture = PBREnvironment::BakeBRDFLUT();
    }
}

void Renderer3D::SetEnvironmentMaps(const X::Ref<TextureCube>& envMap, const X::Ref<TextureCube>& irradianceMap,
                                    const X::Ref<TextureCube>& prefilterMap, uint32_t brdfLUTTexture) {
    s_data.EnvironmentMap = envMap;
    s_data.IrradianceMap = irradianceMap;
    s_data.PrefilterMap = prefilterMap;
    s_data.BRDFLUTTexture = brdfLUTTexture;
}

void Renderer3D::SetExposure(float exposure) {
    s_data.PBRBuffer.Exposure = exposure;
}

const X::Ref<TextureCube>& Renderer3D::GetEnvironmentMap() {
    return s_data.EnvironmentMap;
}

void Renderer3D::SetLightDirection(const glm::vec3& direction) {
    s_data.LightBuffer.Direction = direction;
}

void Renderer3D::SetLightColor(const glm::vec3& color) {
    s_data.LightBuffer.Diffuse = color;
}

Renderer3D::Statistics Renderer3D::GetStats() {
    return s_data.Stats;
}

void Renderer3D::ResetStats() {
    s_data.Stats = {};
}
