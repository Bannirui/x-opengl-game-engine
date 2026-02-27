//
// Created by rui ding on 2026/2/27.
//

#pragma once

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const   = 0;
    virtual void Unbind() const = 0;

    /**
     * @param vertices float array
     * @param size how many bytes of the array
     * @return
     */
    static VertexBuffer *Create(float *vertices, uint32_t size);
};

class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    virtual void Bind() const   = 0;
    virtual void Unbind() const = 0;

    // how many vertex in the index array
    virtual uint32_t GetCount() const = 0;

    static IndexBuffer *Create(uint32_t *indices, uint32_t size);
};
