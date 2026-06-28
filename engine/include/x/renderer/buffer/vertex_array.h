//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"

#include <vector>

class IndexBuffer;
class VertexBuffer;

/**
 * 封装VAO(vertex array object)
 * 跟VBO一样也是OpenGL抽象的状态 会在显存上开辟空间存储一堆数据 OpenGL会分配唯一的id引用它
 * VAO的本质就是让shader的attribute知道怎么用VBO
 */
class VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
    /**
     * 把EBO告诉VAO 让VAO知道取哪些顶点的索引
     * @param indexBuffer EBO
     */
    virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

    virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

    static Ref<VertexArray> Create();
};
