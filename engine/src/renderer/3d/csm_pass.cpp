//
// Created by rui ding on 2026/6/1.
//

#include "csm_pass.h"

#include "renderer/3d/renderer_3D_internal.h"
#include "x/renderer/vertex_array.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

CSMPass::CSMPass(const std::string& name, uint32_t shadowSize,
                 const std::vector<MeshDrawCommand>& drawCommands)
    : RenderPass(name), m_shadowSize(shadowSize), m_drawCommands(drawCommands) {}

void CSMPass::Setup() {
    WriteResource("CSMShadowMaps");
    m_shader = Shader::Create("asset/shader/ShadowMap.glsl");

    // Create shadow map array (one per cascade)
    for (uint32_t i = 0; i < CASCADE_COUNT; ++i) {
        glGenTextures(1, &m_shadowMaps[i]);
        glBindTexture(GL_TEXTURE_2D, m_shadowMaps[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_shadowSize, m_shadowSize, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    }

    glGenFramebuffers(1, &m_shadowFBO);
}

void CSMPass::Execute() {
    glViewport(0, 0, m_shadowSize, m_shadowSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glCullFace(GL_FRONT);
    m_shader->Bind();

    for (uint32_t cascade = 0; cascade < CASCADE_COUNT; ++cascade) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMaps[cascade], 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        m_shader->SetMat4("u_LightViewProjection", m_lightViewProjections[cascade]);

        for (auto& cmd : m_drawCommands) {
            m_shader->SetMat4("u_Model", cmd.Transform);
            cmd.MeshAsset->GetVertexArray()->Bind();
            glDrawElements(GL_TRIANGLES, cmd.MeshAsset->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        }
    }

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CSMPass::UpdateCascades(const glm::mat4& view, const glm::mat4& projection,
                              const glm::vec3& lightDirection, float nearClip, float farClip) {
    m_nearClip = nearClip;
    m_farClip = farClip;
    m_cascadeSplits = ComputeCascadeSplits(nearClip, farClip);

    glm::mat4 invVP = glm::inverse(projection * view);

    float prevSplit = nearClip;
    for (uint32_t i = 0; i < CASCADE_COUNT; ++i) {
        float splitDist = m_cascadeSplits[i];

        // Get frustum corners in world space
        std::array<glm::vec4, 8> corners;
        float farZ = splitDist;
        int idx = 0;
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z : {0, 1}) {
                    glm::vec4 pt = invVP * glm::vec4(x, y, z ? farZ : prevSplit, 1.0f);
                    corners[idx++] = pt / pt.w;
                }

        // Compute bounding sphere center
        glm::vec3 center(0.0f);
        for (auto& c : corners) center += glm::vec3(c);
        center /= 8.0f;

        // Compute radius
        float radius = 0.0f;
        for (auto& c : corners) radius = std::max(radius, glm::length(glm::vec3(c) - center));
        radius = std::ceil(radius * 16.0f) / 16.0f;

        // Build light view-projection
        glm::vec3 lightDir = glm::normalize(lightDirection);
        glm::vec3 up = glm::abs(lightDir.y) < 0.999f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        glm::mat4 lightView = glm::lookAt(center - lightDir * radius, center, up);
        glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius, 0.0f, radius * 2.0f);

        m_lightViewProjections[i] = lightProj * lightView;
        prevSplit = splitDist;
    }
}

uint32_t CSMPass::GetShadowMapID(uint32_t cascade) const {
    return (cascade < CASCADE_COUNT) ? m_shadowMaps[cascade] : 0;
}
