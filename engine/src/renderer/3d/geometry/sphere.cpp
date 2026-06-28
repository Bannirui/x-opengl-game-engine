//
// Created by rui ding on 2026/6/7.
//

#include "x/renderer/3d/generator.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

Ref<Mesh> GeometryGenerator::CreateSphere(float radius, uint32_t sectors, uint32_t stacks) {
    std::vector<StaticMeshVertex> vertices;
    std::vector<uint32_t> indices;

    float sectorStep = 2.0f * glm::pi<float>() / static_cast<float>(sectors);
    float stackStep = glm::pi<float>() / static_cast<float>(stacks);

    for (uint32_t i = 0; i <= stacks; ++i) {
        float stackAngle = glm::pi<float>() / 2.0f - static_cast<float>(i) * stackStep;
        float xz = radius * std::cos(stackAngle);
        float y = radius * std::sin(stackAngle);

        for (uint32_t j = 0; j <= sectors; ++j) {
            float sectorAngle = static_cast<float>(j) * sectorStep;

            float x = xz * std::cos(sectorAngle);
            float z = xz * std::sin(sectorAngle);

            float nx = std::cos(stackAngle) * std::cos(sectorAngle);
            float nz = std::cos(stackAngle) * std::sin(sectorAngle);

            StaticMeshVertex vertex;
            vertex.Position = {x, y, z};
            vertex.Normal = {nx, std::sin(stackAngle), nz};
            vertex.TexCoord = {
                static_cast<float>(j) / static_cast<float>(sectors),
                static_cast<float>(i) / static_cast<float>(stacks),
            };
            vertices.push_back(vertex);
        }
    }

    for (uint32_t i = 0; i < stacks; ++i) {
        for (uint32_t j = 0; j < sectors; ++j) {
            uint32_t first = i * (sectors + 1) + j;
            uint32_t second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    return Mesh::Create(vertices, indices);
}
