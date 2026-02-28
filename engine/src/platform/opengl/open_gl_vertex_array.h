//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/vertex_array.h"

class OpenGLVertexArray : public VertexArray
{
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray() override;

    void Bind() const override;
    void Unbind() const override;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

    const std::vector<std::shared_ptr<VertexBuffer>> &GetVertexBuffers() const override { return m_vertexBuffers; }
    const std::shared_ptr<IndexBuffer>               &GetIndexBuffer() const override { return m_indexBuffer; }

private:
    uint32_t                                   m_rendererID;     // VAO
    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;  // VAO管理着的VBO
    std::shared_ptr<IndexBuffer>               m_indexBuffer;    // VAO管理着的EBO
    uint32_t                                   m_vertexBufferIndex{0};
};
