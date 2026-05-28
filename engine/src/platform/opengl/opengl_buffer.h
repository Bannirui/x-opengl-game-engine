//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/buffer.h"

// VBO
class OpenGLVertexBuffer : public VertexBuffer {
public:
    /**
     * @param vertices VBO要放的顶点数据 在内存上的地址
     * @param size 这些数据最终要放在显存 也就是需要要显存上开辟多大空间 多少个字节
     */
    OpenGLVertexBuffer(float* vertices, uint32_t size);
    /**
     * 空VBO 先不放数据 后面再放
     * @param size 要在显存上开辟多大空间 多少个字节
     */
    OpenGLVertexBuffer(uint32_t size);

    ~OpenGLVertexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    const BufferLayout& GetLayout() const override {
        return m_bufferLayout;
    }

    void SetLayout(const BufferLayout& layout) override {
        m_bufferLayout = layout;
    }

    void SetData(const void* data, uint32_t size) override;

private:
    // 从GPU显存申请到的连续内存地址 用来放VBO顶点数据用 显存对应的唯一id
    uint32_t m_rendererID;
    // 顶点数据是怎么布局的
    BufferLayout m_bufferLayout;
};

// EBO
class OpenGLIndexBuffer : public IndexBuffer {
public:
    OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
    ~OpenGLIndexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    uint32_t GetCount() const override {
        return m_count;
    }

private:
    uint32_t m_rendererID;
    uint32_t m_count;
};