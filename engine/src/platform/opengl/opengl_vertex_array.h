//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/buffer/vertex_array.h"

/**
 * 为了让shader的attribute怎么取VBO里面的数据
 *   - 首先创建一个VBO 在每次渲染前用glEnableVertexAttribArray通知一下OpenGL
 *   - glVertexAttribPointer告诉OpenGL怎么理解VBO里面的每个顶点
 */
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
    /**
     * OpenGL封装了VAO(vertex array object) 在显存开辟对应空间 OpenGL分配唯一id引用它
     */
    uint32_t m_rendererID;
    std::vector<X::Ref<VertexBuffer>> m_vertexBuffers;  // VAO管理着的VBO
    X::Ref<IndexBuffer> m_indexBuffer;                  // VAO管理着的EBO
    /**
     * 哪个顶点分量 要让shader的哪个attribute知道取哪些数据
     *   - pos就是0->映射shader里面的location=0->就是shader里面变量a_Position
     *   - color就是1->映射shader里面的location=1->就是shader里面变量a_Color
     */
    uint32_t m_vertexBufferIndex{0};
};
