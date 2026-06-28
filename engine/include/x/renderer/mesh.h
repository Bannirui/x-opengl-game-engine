//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

class VertexArray;
class VertexBuffer;
class IndexBuffer;

/**
 * 网格顶点的数据格式
 * 每个顶点8个float 共32字节 GPU会按照这个布局解读VBO里面的数据
 */
struct StaticMeshVertex {
    // 位置 x y z 它的offset是0
    glm::vec3 Position;
    // 法线 nx ny nz 它的offset是12
    glm::vec3 Normal;
    // 纹理坐标 u v 它的offset是24
    glm::vec2 TexCoord;
};

/**
 * 一个可以渲染几何体的完整GPU数据封装
 * 代表的是一个准备好可以画的3D物体形状
 */
class Mesh {
private:
    Mesh() = default;

public:
    /**
     * 工厂函数
     * 传入CPU侧内存顶点+索引数组 内部创建VBO EBO VAO三件套
     * 设置布局返回一个万事俱备就等画的Mesh
     * @param vertices 内存上的顶点数据
     * @param indices 内存上的索引数组
     * @return 坐等直接提交给GPU绘制的几何数据
     */
    static Ref<Mesh> Create(const std::vector<StaticMeshVertex>& vertices, const std::vector<uint32_t>& indices);

    const Ref<VertexArray>& GetVertexArray() const {
        return m_vertexArray;
    }

    uint32_t GetIndexCount() const {
        return m_indexCount;
    }

private:
    // 串联VBO+EBO的一个说明书 告诉GPU顶点数据怎么解读
    Ref<VertexArray> m_vertexArray;
    // 显存上的顶点数据块 位置 法线 UV
    Ref<VertexBuffer> m_vertexBuffer;
    // 显存上的索引数组 复用顶点
    Ref<IndexBuffer> m_indexBuffer;
    // 索引总数据 绘制的时候传给glDrawElements
    uint32_t m_indexCount = 0;
};
