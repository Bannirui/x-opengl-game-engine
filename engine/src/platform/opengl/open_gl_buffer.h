//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/buffer.h"

// VBO
class OpenGLVertexBuffer : public VertexBuffer
{
public:
    OpenGLVertexBuffer(float *vertices, uint32_t size);
    ~OpenGLVertexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    const BufferLayout &GetLayout() const override { return m_bufferLayout; }
    void                SetLayout(const BufferLayout &layout) override { m_bufferLayout = layout; }

private:
    uint32_t     m_rendererID;
    BufferLayout m_bufferLayout;  // 成员类型是值
};

// EBO
class OpenGLIndexBuffer : public IndexBuffer
{
public:
    OpenGLIndexBuffer(uint32_t *indices, uint32_t count);
    ~OpenGLIndexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    uint32_t GetCount() const override { return m_count; }

private:
    uint32_t m_rendererID;
    uint32_t m_count;
};