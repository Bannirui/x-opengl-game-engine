//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/mesh.h"

#include "x/renderer/buffer.h"
#include "x/renderer/vertex_array.h"

X::Ref<Mesh> Mesh::Create(const std::vector<StaticMeshVertex>& vertices, const std::vector<uint32_t>& indices) {
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(!vertices.empty(), "Mesh must have vertices");
    X_CORE_ASSERT(!indices.empty(), "Mesh must have indices");

    X::Ref<Mesh> mesh(new Mesh());
    // 内存上的顶点
    mesh->m_vertexBuffer = VertexBuffer::Create((float*)vertices.data(),
                                                static_cast<uint32_t>(vertices.size() * sizeof(StaticMeshVertex)));
    BufferLayout layout = {
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat3, "a_Normal"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
    };
    // 顶点的布局
    mesh->m_vertexBuffer->SetLayout(layout);
    // 索引数组
    mesh->m_indexBuffer = IndexBuffer::Create((uint32_t*)indices.data(), static_cast<uint32_t>(indices.size()));
    // VAO
    mesh->m_vertexArray = VertexArray::Create();
    // VAO关联的VBO
    mesh->m_vertexArray->AddVertexBuffer(mesh->m_vertexBuffer);
    // VAO关联的EBO
    mesh->m_vertexArray->SetIndexBuffer(mesh->m_indexBuffer);
    mesh->m_indexCount = static_cast<uint32_t>(indices.size());
    return mesh;
}
