//
// Created by rui ding on 2026/6/1.
//

#include "advanced_effects.h"

#include "renderer/3d/renderer_3D_internal.h"
#include "x/renderer/vertex_array.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <random>

// ---- Fullscreen Quad ----

static GLuint s_quadVAO = 0;
static GLuint s_quadVBO = 0;

void FullscreenQuad::Init() {
    float verts[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    glGenVertexArrays(1, &s_quadVAO);
    glGenBuffers(1, &s_quadVBO);
    glBindVertexArray(s_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void FullscreenQuad::Render() {
    glBindVertexArray(s_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint FullscreenQuad::GetVAO() {
    return s_quadVAO;
}

// ---- Helper: create 2D texture ----

static uint32_t createTexture2D(uint32_t w, uint32_t h, GLenum internalFormat, GLenum format, GLenum type) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

// ---- Shadow Map Pass ----

ShadowMapPass::ShadowMapPass(const std::string& name, uint32_t shadowSize,
                             const std::vector<MeshDrawCommand>& drawCommands)
    : RenderPass(name), m_shadowSize(shadowSize), m_drawCommands(drawCommands) {}

void ShadowMapPass::Setup() {
    WriteResource("ShadowMap");
    m_shader = Shader::Create("asset/shader/ShadowMap.glsl");

    // Create depth-only FBO
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_shadowSize, m_shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glGenFramebuffers(1, &m_shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapPass::Execute() {
    glViewport(0, 0, m_shadowSize, m_shadowSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    m_shader->Bind();
    m_shader->SetMat4("u_LightViewProjection", m_lightViewProjection);

    for (auto& cmd : m_drawCommands) {
        m_shader->SetMat4("u_Model", cmd.Transform);
        cmd.MeshAsset->GetVertexArray()->Bind();
        glDrawElements(GL_TRIANGLES, cmd.MeshAsset->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapPass::SetLightViewProjection(const glm::mat4& vp) {
    m_lightViewProjection = vp;
}

uint32_t ShadowMapPass::GetShadowMapID() const {
    GLint tex = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                          &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return tex;
}

// ---- SSAO Pass ----

SSAOPass::SSAOPass(const std::string& name, uint32_t width, uint32_t height)
    : RenderPass(name), m_width(width), m_height(height) {
    // Generate hemisphere kernel
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    std::default_random_engine gen;
    for (int i = 0; i < 64; ++i) {
        glm::vec3 sample(rnd(gen) * 2.0f - 1.0f, rnd(gen) * 2.0f - 1.0f, rnd(gen));
        sample = glm::normalize(sample) * rnd(gen);
        float scale = float(i) / 64.0f;
        sample *= glm::mix(0.1f, 1.0f, scale * scale);
        m_ssaoKernel.push_back(sample);
    }
}

void SSAOPass::Setup() {
    WriteResource("SSAO");
    m_ssaoShader = Shader::Create("asset/shader/SSAO.glsl");
    m_blurShader = Shader::Create("asset/shader/SSAOBlur.glsl");
}

void SSAOPass::Execute() {
    GLuint ssaoTex = createTexture2D(m_width / 2, m_height / 2, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
    GLuint blurTex = createTexture2D(m_width / 2, m_height / 2, GL_R8, GL_RED, GL_UNSIGNED_BYTE);

    GLuint ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTex, 0);

    glViewport(0, 0, m_width / 2, m_height / 2);
    glClear(GL_COLOR_BUFFER_BIT);

    m_ssaoShader->Bind();
    m_ssaoShader->SetMat4("u_Projection", m_projection);
    m_ssaoShader->SetFloat("u_Radius", 0.5f);
    m_ssaoShader->SetFloat("u_Bias", 0.025f);
    for (size_t i = 0; i < m_ssaoKernel.size(); ++i) {
        m_ssaoShader->SetFloat3("u_Samples[" + std::to_string(i) + "]", m_ssaoKernel[i]);
    }
    FullscreenQuad::Render();

    // Blur pass
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex, 0);
    m_blurShader->Bind();
    m_blurShader->SetFloat2("u_TexelSize", {1.0f / (m_width / 2), 1.0f / (m_height / 2)});
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoTex);
    m_blurShader->SetInt("u_SSAOInput", 0);
    FullscreenQuad::Render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &ssaoTex);
    glDeleteFramebuffers(1, &ssaoFBO);

    // Store result (external resource for material to sample)
    // Note: in practice, blurTex would be stored and reused across frames.
    // This simplified version recreates it each frame.
    glDeleteTextures(1, &blurTex);
}

void SSAOPass::SetProjection(const glm::mat4& proj) {
    m_projection = proj;
}

uint32_t SSAOPass::GetSSAOTextureID() const {
    return 0;  // Would need to cache for reuse
}

// ---- Bloom Pass ----

BloomPass::BloomPass(const std::string& name, uint32_t width, uint32_t height)
    : RenderPass(name), m_width(width), m_height(height) {}

void BloomPass::Setup() {
    WriteResource("Bloom");
    m_extractShader = Shader::Create("asset/shader/BloomExtract.glsl");
    m_blurShader = Shader::Create("asset/shader/BloomBlur.glsl");
    m_compositeShader = Shader::Create("asset/shader/BloomComposite.glsl");

    uint32_t hw = m_width / 2, hh = m_height / 2;
    m_pingPongA = createTexture2D(hw, hh, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    m_pingPongB = createTexture2D(hw, hh, GL_RGBA16F, GL_RGBA, GL_FLOAT);
}

void BloomPass::Execute() {
    uint32_t hw = m_width / 2, hh = m_height / 2;
    GLuint fbo;
    glGenFramebuffers(1, &fbo);

    // Extract bright areas
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingPongA, 0);
    glViewport(0, 0, hw, hh);
    glClear(GL_COLOR_BUFFER_BIT);
    m_extractShader->Bind();
    m_extractShader->SetFloat("u_Threshold", m_threshold);
    FullscreenQuad::Render();

    // Blur passes (5 iterations, ping-pong)
    m_blurShader->Bind();
    m_blurShader->SetFloat2("u_TexelSize", {1.0f / hw, 1.0f / hh});
    for (int i = 0; i < 10; ++i) {
        bool horizontal = (i % 2 == 0);
        GLuint readTex = horizontal ? m_pingPongA : m_pingPongB;
        GLuint writeTex = horizontal ? m_pingPongB : m_pingPongA;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, writeTex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, readTex);
        m_blurShader->SetInt("u_Image", 0);
        m_blurShader->SetInt("u_Horizontal", horizontal ? 1 : 0);
        FullscreenQuad::Render();
    }

    // Composite
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
    m_compositeShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_pingPongB);  // Bloom result
    m_compositeShader->SetInt("u_Bloom", 0);
    m_compositeShader->SetFloat("u_Exposure", m_exposure);
    FullscreenQuad::Render();

    glDeleteFramebuffers(1, &fbo);
}

uint32_t BloomPass::GetBloomTextureID() const {
    return m_pingPongB;
}
