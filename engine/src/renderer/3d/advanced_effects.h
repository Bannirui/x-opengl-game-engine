//
// Created by rui ding on 2026/6/1.
// Advanced rendering effects as RenderPass implementations
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/mesh.h"
#include "x/renderer/render_graph.h"
#include "x/renderer/shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <vector>

struct MeshDrawCommand;

// ---- Shadow Map Pass ----
class ShadowMapPass : public RenderPass {
public:
    ShadowMapPass(const std::string& name, uint32_t shadowSize, const std::vector<MeshDrawCommand>& drawCommands);

    void Setup() override;
    void Execute() override;

    void SetLightViewProjection(const glm::mat4& vp);
    uint32_t GetShadowMapID() const;

private:
    uint32_t m_shadowSize;
    const std::vector<MeshDrawCommand>& m_drawCommands;
    glm::mat4 m_lightViewProjection{1.0f};
    X::Ref<Shader> m_shader;
    uint32_t m_shadowFBO = 0;
};

// ---- SSAO Pass ----
class SSAOPass : public RenderPass {
public:
    SSAOPass(const std::string& name, uint32_t width, uint32_t height);
    void Setup() override;
    void Execute() override;

    void SetProjection(const glm::mat4& proj);
    uint32_t GetSSAOTextureID() const;

private:
    uint32_t m_width, m_height;
    glm::mat4 m_projection{1.0f};
    X::Ref<Shader> m_ssaoShader, m_blurShader;
    std::vector<glm::vec3> m_ssaoKernel;
};

// ---- Bloom Pass ----
class BloomPass : public RenderPass {
public:
    BloomPass(const std::string& name, uint32_t width, uint32_t height);
    void Setup() override;
    void Execute() override;

    uint32_t GetBloomTextureID() const;

    void SetThreshold(float t) {
        m_threshold = t;
    }

    void SetExposure(float e) {
        m_exposure = e;
    }

private:
    uint32_t m_width, m_height;
    float m_threshold = 1.0f;
    float m_exposure = 1.0f;
    X::Ref<Shader> m_extractShader, m_blurShader, m_compositeShader;
    uint32_t m_pingPongA = 0, m_pingPongB = 0;
};

// ---- Fullscreen Quad Utility ----
class FullscreenQuad {
public:
    static void Init();
    static void Render();
    static GLuint GetVAO();
};
