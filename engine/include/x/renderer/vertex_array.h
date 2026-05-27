//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"

#include <vector>

class IndexBuffer;
class VertexBuffer;

// VAO
class VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const X::Ref<VertexBuffer>& vertexBuffer) = 0;
    /**
     * 把EBO告诉VAO 让VAO知道取哪些顶点的索引
     * @param indexBuffer EBO
     */
    virtual void SetIndexBuffer(const X::Ref<IndexBuffer>& indexBuffer) = 0;

    virtual const std::vector<X::Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const X::Ref<IndexBuffer>& GetIndexBuffer() const = 0;

    static X::Ref<VertexArray> Create();
};
