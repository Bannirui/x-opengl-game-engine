//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"
#include "x/core.h"

class IndexBuffer;
class VertexBuffer;

// VAO
class VertexArray
{
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const   = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer)    = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer>> &GetVertexBuffers() const = 0;
    virtual const std::shared_ptr<IndexBuffer>               &GetIndexBuffer() const   = 0;

    static X::Ref<VertexArray> Create();
};
