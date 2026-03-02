//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/buffer.h"

// VBO
class OpenGLVertexBuffer : public VertexBuffer
{
public:
    /**
     * @param vertices VBO要放的顶点数据
     * @param size 顶点数据多大
     */
    OpenGLVertexBuffer(float *vertices, uint32_t size);
    /**
     * 空VBO 先不放数据 后面再放
     * @param size 顶点数据多大
     */
    OpenGLVertexBuffer(uint32_t size);

    ~OpenGLVertexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    const BufferLayout &GetLayout() const override { return m_bufferLayout; }
    void                SetLayout(const BufferLayout &layout) override { m_bufferLayout = layout; }

    void SetData(const void *data, uint32_t size) override;

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