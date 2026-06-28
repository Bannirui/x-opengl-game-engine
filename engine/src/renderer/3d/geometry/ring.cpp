//
// Created by rui ding on 2026/6/7.
//

#include "x/renderer/3d/generator.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

Ref<Mesh> GeometryGenerator::CreateRing(float innerRadius, float outerRadius, uint32_t segments) {
    std::vector<StaticMeshVertex> vertices;
    std::vector<uint32_t> indices;

    float angleStep = 2.0f * glm::pi<float>() / static_cast<float>(segments);

    for (uint32_t i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) * angleStep;
        float x = std::cos(angle);
        float z = std::sin(angle);

        StaticMeshVertex inner;
        inner.Position = {x * innerRadius, 0.0f, z * innerRadius};
        inner.Normal = {0.0f, 1.0f, 0.0f};
        inner.TexCoord = {0.0f, 0.0f};
        vertices.push_back(inner);

        StaticMeshVertex outer;
        outer.Position = {x * outerRadius, 0.0f, z * outerRadius};
        outer.Normal = {0.0f, 1.0f, 0.0f};
        outer.TexCoord = {0.0f, 0.0f};
        vertices.push_back(outer);
    }

    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t inner0 = i * 2;
        uint32_t outer0 = i * 2 + 1;
        uint32_t inner1 = (i + 1) * 2;
        uint32_t outer1 = (i + 1) * 2 + 1;

        indices.push_back(inner0);
        indices.push_back(outer0);
        indices.push_back(inner1);

        indices.push_back(outer0);
        indices.push_back(outer1);
        indices.push_back(inner1);
    }

    return Mesh::Create(vertices, indices);
}
