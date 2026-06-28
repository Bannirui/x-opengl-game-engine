//
// Created by rui ding on 2026/6/1.
// Cascaded Shadow Maps pass
//

#pragma once

#include "x/core/base.h"

#include "x/renderer/render_graph.h"
#include "x/renderer/shader.h"

#include <glm/glm.hpp>

#include <vector>

struct MeshDrawCommand;

class CSMPass : public RenderPass {
public:
    static constexpr uint32_t CASCADE_COUNT = 4;

    CSMPass(const std::string& name, uint32_t shadowSize,
            const std::vector<MeshDrawCommand>& drawCommands);

    void Setup() override;
    void Execute() override;

    void UpdateCascades(const glm::mat4& view, const glm::mat4& projection,
                        const glm::vec3& lightDirection, float nearClip, float farClip);

    uint32_t GetShadowMapID(uint32_t cascade) const;
    const glm::mat4& GetLightViewProjection(uint32_t cascade) const {
        return m_lightViewProjections[cascade];
    }
    const std::array<float, CASCADE_COUNT>& GetCascadeSplits() const { return m_cascadeSplits; }

private:
    uint32_t m_shadowSize;
    const std::vector<MeshDrawCommand>& m_drawCommands;
    Ref<Shader> m_shader;
    uint32_t m_shadowFBO = 0;
    uint32_t m_shadowMaps[CASCADE_COUNT] = {};
    glm::mat4 m_lightViewProjections[CASCADE_COUNT]{};
    std::array<float, CASCADE_COUNT> m_cascadeSplits{};
    float m_nearClip = 0.1f;
    float m_farClip = 500.0f;
};

inline std::array<float, CSMPass::CASCADE_COUNT> ComputeCascadeSplits(float nearClip, float farClip,
                                                                       float lambda = 0.75f) {
    std::array<float, CSMPass::CASCADE_COUNT> splits;
    for (uint32_t i = 0; i < CSMPass::CASCADE_COUNT; ++i) {
        float p = (i + 1) / float(CSMPass::CASCADE_COUNT);
        float log = nearClip * std::pow(farClip / nearClip, p);
        float uniform = nearClip + (farClip - nearClip) * p;
        splits[i] = lambda * log + (1.0f - lambda) * uniform;
    }
    return splits;
}
