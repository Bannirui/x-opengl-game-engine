//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

class VertexArray;
class VertexBuffer;
class IndexBuffer;

struct StaticMeshVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

class Mesh {
public:
    static X::Ref<Mesh> Create(const std::vector<StaticMeshVertex>& vertices,
                               const std::vector<uint32_t>& indices);

    const X::Ref<VertexArray>& GetVertexArray() const { return m_vertexArray; }
    uint32_t GetIndexCount() const { return m_indexCount; }

    Mesh() = default;

public:

    X::Ref<VertexArray> m_vertexArray;
    X::Ref<VertexBuffer> m_vertexBuffer;
    X::Ref<IndexBuffer> m_indexBuffer;
    uint32_t m_indexCount = 0;
};
