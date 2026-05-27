//
// Created by dingrui on 5/26/26.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/buffer.h"
#include "x/renderer/vertex_array.h"

/**
 * 批次化渲染组 管理一种图形的完整渲染管线
 *
 * GPU层面 VBO预先申请开辟n个顶点容量的显存 init的时候申请好
 * CPU层面 [Base...Ptr)堆内存表示实际放的顶点数据 flush的时候数据灌给GPU
 *
 * 这个设计就是在CPU侧用个缓存来放每帧要画什么
 *   - 在绘制每帧之前先清空这个缓存内容 用指针控制在逻辑上的清空 并不负责真正的空间释放
 *   - 每次绘制就先把所有有画的顶点数据放到这个缓存
 *   - 都放完后一起传给GPU的显存并调用绘制的API
 *     - DrawArrays
 *     - DrawElements
 *
 * @tparam VertexType 顶点的结构
 * @tparam VerticesPerDraw 每次绘制用几个顶点
 *                           - 矩形 4个顶点 两个三角形 将来用DrawElements方式画
 *                           - 圆形 4个顶点 两个三角形 将来用DrawArrays方式画
 *                           - 线段 2个顶点
 * @tparam UseIndex 底层API绘制图形的方式 是否用IBO索引绘制
 *                  用IBO Init的时候生成静态的IBO 每4个顶点就生成6个索引 将来用DrawElements方式画
 *                  不用IBO 不生成IBO 将来用DrawArrays方式画
 */
template <typename VertexType, uint32_t VerticesPerDraw, bool UseIndex = true>
struct BatchGroup {
    // 缓存最多放多少个图形的信息
    uint32_t MaxBatches = 20000;

    /**
     * 比如 1个图形4个顶点
     * 也就是VBO有多少个
     * @return 整个缓存放了多少个顶点
     */
    uint32_t MaxVertices() const {
        return MaxBatches * VerticesPerDraw;
    }

    /**
     * 比如 1个图形4个顶点画三角形就行要画两个三角形 每个三角形3个顶点的索引 就是有6个顶点索引
     * 也就是VAO有多少个
     * @return 整个缓存放了多少个索引
     */
    uint32_t MaxIndices() const {
        return MaxBatches * (UseIndex ? VerticesPerDraw / 4 * 6 : VerticesPerDraw);
    }

    // 顶点的索引信息
    X::Ref<VertexArray> VAO;
    // 顶点信息
    X::Ref<VertexBuffer> VBO;
    /**
     * 双指针管理缓存空间
     * CPU侧内存上[Base...Ptr)区间维护着实际的顶点数据内存
     *   - 每次绘制前 Ptr移动到底部就相当于逻辑清空了缓存空间
     *   - 每次绘制的时候 就通过移动Ptr往缓存写数据 要绘制图形的顶点信息和索引信息
     *   - 真正绘制的时候 提交给GPU的时候要是告诉GPU显存顶点数据在内存的什么地方有多少个字节
     */
    VertexType* Base{nullptr};
    VertexType* Ptr{nullptr};
    /**
     * 批量提交给GPU的有多少个顶点
     * 对应的并不是真正物理上的顶点个数
     *   - 用DrawArrays时有多少个顶点 这个count就有多少个 对应的就是物理上的顶点个数
     *   - 用DrawElements时会重复使用顶点 这个count就是看逻辑上多少个顶点 就是用了几个顶点的索引来画图形
     */
    uint32_t Count{0};

    void Init(const BufferLayout& layout) {
        VAO = VertexArray::Create();
        VBO = VertexBuffer::Create(MaxVertices() * sizeof(VertexType));
        VBO->SetLayout(layout);
        VAO->AddVertexBuffer(VBO);
        Base = new VertexType[MaxVertices()];

        if constexpr (UseIndex) {
            auto indices = std::make_unique<uint32_t[]>(MaxIndices());
            uint32_t offset = 0;
            for (uint32_t i = 0; i < MaxIndices(); i += 6) {
                indices[i + 0] = offset + 0;
                indices[i + 1] = offset + 1;
                indices[i + 2] = offset + 2;
                indices[i + 3] = offset + 2;
                indices[i + 4] = offset + 3;
                indices[i + 5] = offset + 0;
                offset += VerticesPerDraw;
            }
            auto ib = IndexBuffer::Create(indices.get(), MaxIndices());
            VAO->SetIndexBuffer(ib);
        }

        Ptr = Base;
        Count = 0;
    }

    void Shutdown() {
        delete[] Base;
        VAO.reset();
        VBO.reset();
    }

    // 每帧绘制前调用 清空缓存里面的数据内容 准备放上这一帧要画的数据
    void StartBatch() {
        Ptr = Base;
        Count = 0;
    }

    bool IsFull() const {
        return Count >= (UseIndex ? MaxIndices() : MaxVertices());
    }

    // CPU侧VBO顶点信息有多少字节
    uint32_t GetDataSize() const {
        return static_cast<uint32_t>(reinterpret_cast<uint8_t*>(Ptr) - reinterpret_cast<uint8_t*>(Base));
    }
};
