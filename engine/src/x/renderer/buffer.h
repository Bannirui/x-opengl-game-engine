//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"

#include "x/core/core.h"
#include "x/core/x_log.h"

enum class ShaderDataType : uint16_t
{
    kNone = 0,
    kFloat,
    kFloat2,
    kFloat3,
    kFloat4,
    kMat3,
    kMat4,
    kInt,
    kInt2,
    kInt3,
    kInt4,
    kBool,
};

// 数据类型占几个字节
static uint32_t ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::kNone:
        {
            return 0;
        }
        case ShaderDataType::kFloat:
        {
            return sizeof(float);
        }
        case ShaderDataType::kFloat2:
        {
            return 2 * sizeof(float);
        }
        case ShaderDataType::kFloat3:
        {
            return 3 * sizeof(float);
        }
        case ShaderDataType::kFloat4:
        {
            return 4 * sizeof(float);
        }
        case ShaderDataType::kMat3:
        {
            return 3 * 3 * sizeof(float);
        }
        case ShaderDataType::kMat4:
        {
            return 4 * 4 * sizeof(float);
        }
        case ShaderDataType::kInt:
        {
            return sizeof(int);
        }
        case ShaderDataType::kInt2:
        {
            return 2 * sizeof(int);
        }
        case ShaderDataType::kInt3:
        {
            return 3 * sizeof(int);
        }
        case ShaderDataType::kInt4:
        {
            return 4 * sizeof(int);
        }
        case ShaderDataType::kBool:
        {
            return 1;
        }
    }
    X_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

struct BufferElement
{
    std::string    name;
    ShaderDataType type;
    uint32_t       size;
    size_t         offset;
    bool           normalized;

    BufferElement() = default;

    BufferElement(ShaderDataType type, const std::string &name, bool normalized = false)
        : name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
    {
    }

    uint32_t GetComponentCount() const
    {
        switch (type)
        {
            case ShaderDataType::kNone:
            {
                return 0;
            }
            case ShaderDataType::kFloat:
            {
                return 1;
            }
            case ShaderDataType::kFloat2:
            {
                return 2;
            }
            case ShaderDataType::kFloat3:
            {
                return 3;
            }
            case ShaderDataType::kFloat4:
            {
                return 4;
            }
            case ShaderDataType::kMat3:
            {
                return 3 * 3;
            }
            case ShaderDataType::kMat4:
            {
                return 4 * 4;
            }
            case ShaderDataType::kInt:
            {
                return 1;
            }
            case ShaderDataType::kInt2:
            {
                return 2;
            }
            case ShaderDataType::kInt3:
            {
                return 3;
            }
            case ShaderDataType::kInt4:
            {
                return 4;
            }
            case ShaderDataType::kBool:
            {
                return 1;
            }
        }
        X_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }
};

class BufferLayout
{
public:
    BufferLayout() {}

    BufferLayout(const std::initializer_list<BufferElement> &elements) : m_elements(elements)
    {
        calculateOffsetsAndStride();
    }

    inline uint32_t                          GetStride() const { return m_stride; }
    inline const std::vector<BufferElement> &GetElements() const { return m_elements; }

    std::vector<BufferElement>::iterator       begin() { return m_elements.begin(); }
    std::vector<BufferElement>::iterator       end() { return m_elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

private:
    void calculateOffsetsAndStride()
    {
        size_t offset = 0;
        m_stride      = 0;
        for (auto &element : m_elements)
        {
            element.offset = offset;
            offset += element.size;
            m_stride += element.size;
        }
    }

private:
    std::vector<BufferElement> m_elements;
    uint32_t                   m_stride = 0;
};

// VBO
class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual const BufferLayout &GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout &layout) = 0;

    /**
     * 往空的VBO里面灌数据
     * @param data 要灌的数据
     * @param size 数据多少个字节
     */
    virtual void SetData(const void* data, uint32_t size) = 0;

    /**
     * @param vertices float array
     * @param size how many bytes of the array
     */
    static X::Ref<VertexBuffer> Create(float *vertices, uint32_t size);
    /**
     * 空的VBO 没有数据
     * @param size VBO要多大空间 后面再放数据
     */
    static X::Ref<VertexBuffer> Create(uint32_t size);
};

// EBO
class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    // how many vertex in the index array
    virtual uint32_t GetCount() const = 0;

    /**
     * @param indices VAO数据
     * @param count EBO里面多少个顶点
     */
    static X::Ref<IndexBuffer> Create(uint32_t *indices, uint32_t count);
};
