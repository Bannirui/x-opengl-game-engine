//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/vertex_array.h"

class OpenGLVertexArray : public VertexArray {
public:
    // 在GPU显存申请一个VAO 空白的显存
    OpenGLVertexArray();
    ~OpenGLVertexArray() override;

    void Bind() const override;
    void Unbind() const override;

    void AddVertexBuffer(const X::Ref<VertexBuffer>& vertexBuffer) override;
    void SetIndexBuffer(const X::Ref<IndexBuffer>& indexBuffer) override;

    const std::vector<X::Ref<VertexBuffer>>& GetVertexBuffers() const override {
        return m_vertexBuffers;
    }

    const X::Ref<IndexBuffer>& GetIndexBuffer() const override {
        return m_indexBuffer;
    }

private:
    uint32_t m_rendererID;                              // VAO
    std::vector<X::Ref<VertexBuffer>> m_vertexBuffers;  // VAO管理着的VBO
    X::Ref<IndexBuffer> m_indexBuffer;                  // VAO管理着的EBO
    /**
     * 哪个顶点分量
     *   - pos就是0->映射shader里面的location=0->就是shader里面变量a_Position
     *   - color就是1->映射shader里面的location=1->就是shader里面变量a_Color
     */
    uint32_t m_vertexBufferIndex{0};
};
