//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"

#include <cstdint>
#include <initializer_list>
#include <vector>

/**
 * VBO的分量类型
 * 什么叫分量 比如VBO顶点数据每个顶点数据包含
 *   - pos
 *   - color
 *   - 法线
 *   - ...
 * 顶点数据里面的pos就是一个分量 color也是一个分量
 */
enum class ShaderDataType : uint16_t {
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
static uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::kNone: {
            return 0;
        }
        case ShaderDataType::kFloat: {
            return sizeof(float);
        }
        case ShaderDataType::kFloat2: {
            return 2 * sizeof(float);
        }
        case ShaderDataType::kFloat3: {
            return 3 * sizeof(float);
        }
        case ShaderDataType::kFloat4: {
            return 4 * sizeof(float);
        }
        case ShaderDataType::kMat3: {
            return 3 * 3 * sizeof(float);
        }
        case ShaderDataType::kMat4: {
            return 4 * 4 * sizeof(float);
        }
        case ShaderDataType::kInt: {
            return sizeof(int);
        }
        case ShaderDataType::kInt2: {
            return 2 * sizeof(int);
        }
        case ShaderDataType::kInt3: {
            return 3 * sizeof(int);
        }
        case ShaderDataType::kInt4: {
            return 4 * sizeof(int);
        }
        case ShaderDataType::kBool: {
            return 1;
        }
    }
    CORE_ERROR("Unknown ShaderDataType!");
    return 0;
}

/**
 * VBO的布局情况
 * VBO里面就是1个或多个顶点
 * 每个顶点由1个或多个分量组成
 *   - pos
 *   - color
 *   - normal
 *   - ...
 * 这么多个分量是什么顺序 每个分量多少个数字 每个数字是什么类型
 * BufferElement就表达一个分量也就是对应shader里面的attribute
 */
struct BufferElement {
    /**
     * 这个分量对应着色器里面location的名字
     *   - a_Position
     *   - a_Color
     *   - ...
     */
    std::string name;
    /**
     * 这个分量有什么数据类型表达的
     *   - pos用3个float
     *   - color用4个float
     */
    ShaderDataType type;
    // 这个分量的数据多少个字节
    uint32_t size;
    /**
     * 每个分量在顶点的偏移是多少
     * 假设顶点      pos            color
     *          x    y    z    r    g    b   a
     *        0.1f 0.2f 0.3f 0.1f 0.2f 0.3f 0.4f
     * 那么
     *   - pos这个分量在顶点的偏移是0
     *   - color这个分量在顶点的偏移是3个float=24字节
     */
    size_t offset;
    // 数据是不是归一化的
    bool normalized;

    BufferElement() = default;

    /**
     * 顶点的分量
     * @param type 分量用的什么数据表达的 比如pos用3个float color用4个float
     * @param name 这个分量对应shader着色器glsl代码里面的变量名 比如a_Position a_Color
     * @param normalized 数据是不是归一化的
     */
    BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
        : name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized) {}

    /**
     * 每个分量都由1个或多个数据组成 比如
     *   - pos有3个float xyz
     *   - color有4个float rgba
     *   - ...
     * @return 分量有几个数据
     */
    uint32_t GetComponentCount() const {
        switch (type) {
            case ShaderDataType::kNone: {
                return 0;
            }
            case ShaderDataType::kFloat: {
                return 1;
            }
            case ShaderDataType::kFloat2: {
                return 2;
            }
            case ShaderDataType::kFloat3: {
                return 3;
            }
            case ShaderDataType::kFloat4: {
                return 4;
            }
            case ShaderDataType::kMat3: {
                return 3;  // 3*float3
            }
            case ShaderDataType::kMat4: {
                return 4;  // 4*float4
            }
            case ShaderDataType::kInt: {
                return 1;
            }
            case ShaderDataType::kInt2: {
                return 2;
            }
            case ShaderDataType::kInt3: {
                return 3;
            }
            case ShaderDataType::kInt4: {
                return 4;
            }
            case ShaderDataType::kBool: {
                return 1;
            }
        }
        CORE_ERROR("Unknown ShaderDataType!");
        return 0;
    }
};

/**
 * VBO的多个attribute布局情况
 * VBO里面就是1个或多个顶点
 * 每个顶点由1个或多个分量组成
 *   - pos
 *   - color
 *   - normal
 *   - ...
 * 这么多个分量是什么顺序 每个分量多少个数字 每个数字是什么类型
 */
class BufferLayout {
public:
    BufferLayout() {}

    BufferLayout(const std::initializer_list<BufferElement>& elements) : m_elements(elements) {
        calculateOffsetsAndStride();
    }

    uint32_t GetStride() const {
        return m_stride;
    }

    const std::vector<BufferElement>& GetElements() const {
        return m_elements;
    }

    std::vector<BufferElement>::iterator begin() {
        return m_elements.begin();
    }

    std::vector<BufferElement>::iterator end() {
        return m_elements.end();
    }

    std::vector<BufferElement>::const_iterator begin() const {
        return m_elements.begin();
    }

    std::vector<BufferElement>::const_iterator end() const {
        return m_elements.end();
    }

private:
    void calculateOffsetsAndStride() {
        size_t offset = 0;
        // 统计顶点数据多少字节 就是顶点里面所有分量大小加起来
        m_stride = 0;
        for (auto& element : m_elements) {
            // 顶点分量在顶点的偏移
            element.offset = offset;
            offset += element.size;
            // 每个分量大小求和
            m_stride += element.size;
        }
    }

private:
    // 一个顶点的分量 放在vector就顺序性就是每个分量的顺序
    std::vector<BufferElement> m_elements;
    // 一个顶点的步长 也就是一个顶点数据多少字节 VBO是GPU显存上一个连续内存空间 一连串的数据
    // GPU不知道这些数据哪些是顶点A 哪些是顶点B 这个步长就是负责告诉GPU每个顶点数据是怎么划分的
    uint32_t m_stride = 0;
};

/**
 * VBO(vertex buffer object)
 * 在显存上内存空间 OpenGL会生成唯一id标识object
 * OpenGL有很多我buffer类型 VBO对应的buffer类型是GL_ARRAY_BUFFER OpenGL提供了API用为绑定buffer object的类型
 * buffer object的作用是作为媒介用来在内存到显存传数据
 * 拷贝数据的时候根据应用场景选择合适的类型
 *   - GL_STREAM_DRAW 数据不变 GPU用的少
 *   - GL_STATIC_DRAW 数据不变 GPU用的多
 *   - GL_DYNAMIC_DRAW 数据经常变 GPU用的多
 */
class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout& layout) = 0;

    /**
     * 把顶点信息从CPU侧的内存灌给GPU侧显存
     * @param data 要灌的数据在内存什么位置 内存地址
     * @param size 要传多少数据 字节
     */
    virtual void SetData(const void* data, uint32_t size) = 0;

    /**
     * 创建VBO(vertex buffer object)
     * 本质就是一片连续的显存空间
     * @param vertices VBO里面放的数据 这些数据在CPU侧内存位置 内存地址
     * @param size 需要多大的显存空间 多少个字节
     */
    static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
    /**
     * 创建VBO
     * 只要分配空的显存 暂时不放数据
     * @param size 需要多大的显存 字节
     */
    static Ref<VertexBuffer> Create(uint32_t size);
};

/**
 * 封装EBO
 * EBO本质也是让OpenGL在显存上开辟一块空间buffer 这块空间放的就是VBO顶点的索引值
 * VBO里面的顶点首先会有多个attribute 每个attribute还有对应的值
 * 如果每需要一个顶点都要定义占用的内存 显存 以及内存往显存拷贝 都是开销
 * 为了利用重复的顶点信息 就有了这样一个机制
 *   - 用VBO告诉OpenGL有哪些顶点 对应的数据在显存哪儿
 *   - 用EBO告诉OpenGL怎么组合这些顶点
 */
class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    // how many vertex in the index array
    virtual uint32_t GetCount() const = 0;

    /**
     * 创建EBO
     * @param indices EBO里面要放的数据 这些数据在内存上的位置 内存地址 把这些内存上数据复制到显存上
     * @param count 要往显存上发送多少个索引顶点 每个索引是一个整数 很容易计算出来内存要复制多少个字节到显存
     */
    static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
};
