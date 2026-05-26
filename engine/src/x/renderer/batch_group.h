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
 * GPU侧VAO + VBO + (可选)IndexBuffer
 *   - VAO 绑定VBO和IBO的布局描述
 *   - VBO 显存中的顶点缓冲区 每帧Flush时上传
 *   - IndexBuffer 仅UseIndex=true时生成 内容固定为重复的0-1-2-2-3-0
 *
 * CPU侧Base + Ptr + Count
 *   - Base CPU端顶点数组(堆内存) 预分配MaxVertices个
 *   - Ptr 当前写入位置的游标 每写入一个顶点Ptr++
 *   - Count 当前批次累积了多少个索引/顶点 供Flush用
 *
 * Flush时 [Base, Ptr)就是本轮要上传的顶点数据
 * StartBatch时 Ptr回到Base Count清零
 *
 * 使用方式
 *   BatchGroup<QuadVertex, 4> quad;
 *   quad.Init({ {kFloat3, "a_Pos"}, ... });
 *   quad.Ptr->field = value; quad.Ptr++;
 *   quad.Count += quad.VerticesPerDraw;  // Quad/Circle: +=4, Line: +=2
 *   if (quad.IsFull()) nextBatch();
 * @tparam VertexType 顶点的结构
 * @tparam VerticesPerDraw 每次绘制用几个顶点
 *                           - 矩形 4个顶点 两个三角形
 *                           - 圆形 4个顶点 两个三角形
 *                           - 线段 2个顶点
 * @tparam UseIndex 底层API绘制图形的方式 是否用IBO索引绘制
 */
template <typename VertexType, uint32_t VerticesPerDraw, bool UseIndex = true>
struct BatchGroup {
    uint32_t MaxBatches = 20000;

    uint32_t MaxVertices() const {
        return MaxBatches * VerticesPerDraw;
    }

    uint32_t MaxIndices() const {
        return MaxBatches * (UseIndex ? VerticesPerDraw / 4 * 6 : VerticesPerDraw);
    }

    X::Ref<VertexArray> VAO;
    X::Ref<VertexBuffer> VBO;
    // 内存上[Base...Ptr)区间维护着实际的顶点数据内存 提交给GPU的时候要是告诉GPU顶点数据在哪儿的
    VertexType* Base{nullptr};
    VertexType* Ptr{nullptr};
    // 批量提交给GPU的有多少个顶点
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

    void StartBatch() {
        Ptr = Base;
        Count = 0;
    }

    bool IsFull() const {
        return Count >= (UseIndex ? MaxIndices() : MaxVertices());
    }

    uint32_t GetDataSize() const {
        return static_cast<uint32_t>(reinterpret_cast<uint8_t*>(Ptr) - reinterpret_cast<uint8_t*>(Base));
    }
};
