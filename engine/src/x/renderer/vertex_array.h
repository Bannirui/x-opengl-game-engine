//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"
#include "x/core/core.h"

class IndexBuffer;
class VertexBuffer;

// VAO
class VertexArray
{
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const   = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const X::Ref<VertexBuffer> &vertexBuffer) = 0;
    virtual void SetIndexBuffer(const X::Ref<IndexBuffer> &indexBuffer)    = 0;

    virtual const std::vector<X::Ref<VertexBuffer>> &GetVertexBuffers() const = 0;
    virtual const X::Ref<IndexBuffer>               &GetIndexBuffer() const   = 0;

    static X::Ref<VertexArray> Create();
};
