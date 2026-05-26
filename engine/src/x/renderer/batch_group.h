//
// Created by dingrui on 5/26/26.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/buffer.h"
#include "x/renderer/vertex_array.h"

/**
 * 批次化渲染组 管理一种图形的完整渲染管线
 *   - Quad
 *   - Circle
 *   ...
 * GPU侧VAO+VBO+IndexBuffer
 *   - VAO 绑定VBO和IBO的布局描述
 *   - VBO 显存中的顶点缓冲区 每帧Flush时上传
 *   - IndexBuffer 索引缓冲区 内容固定为重复的0-1-2-2-3-0 Init时生成一次
 *
 * CPU侧Base+Ptr+IndexCount
 *   - Base CPU 端顶点数组 堆内存 预分配MaxVertices个
 *   - Ptr 当前写入位置的游标 每写入一个顶点Ptr++
 *   - IndexCount 当前批次累积了多少个索引 供Flush用
 *
 * Flush时 Base到Ptr之间的[Base, Ptr)就是本轮要上传的顶点数据
 * StartBatch时 Ptr回到Base IndexCount清零
 *
 * 使用方式
 *   - BatchGroup<QuadVertex> quad;
 *   - quad.Init({ {kFloat3, "a_Pos"}, ... });      // 创建 VAO+VBO+IBO，分配 CPU 缓冲区
 *   - quad.Ptr->field = value; quad.Ptr++;          // 写入顶点
 *   - quad.IndexCount += 6;                         // 每个 Quad 占 4 顶点 6 索引
 *   - if (quad.IsFull()) nextBatch();               // 批次满则先 Flush
 */
template <typename VertexType>
struct BatchGroup {
    uint32_t MaxQuads = 20000;

    uint32_t MaxVertices() const { return MaxQuads * 4; }
    uint32_t MaxIndices() const { return MaxQuads * 6; }

    X::Ref<VertexArray>  VAO;
    X::Ref<VertexBuffer> VBO;
    VertexType*          Base{nullptr};
    VertexType*          Ptr{nullptr};
    uint32_t             IndexCount{0};

    void Init(const BufferLayout& layout) {
        VAO = VertexArray::Create();
        VBO = VertexBuffer::Create(MaxVertices() * sizeof(VertexType));
        VBO->SetLayout(layout);
        VAO->AddVertexBuffer(VBO);
        Base = new VertexType[MaxVertices()];

        auto     indices = std::make_unique<uint32_t[]>(MaxIndices());
        uint32_t offset  = 0;
        for (uint32_t i = 0; i < MaxIndices(); i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }
        auto ib = IndexBuffer::Create(indices.get(), MaxIndices());
        VAO->SetIndexBuffer(ib);

        Ptr        = Base;
        IndexCount = 0;
    }

    void Shutdown() {
        delete[] Base;
        VAO.reset();
        VBO.reset();
    }

    void StartBatch() {
        Ptr        = Base;
        IndexCount = 0;
    }

    bool IsFull() const { return IndexCount >= MaxIndices(); }

    uint32_t GetDataSize() const {
        return static_cast<uint32_t>(reinterpret_cast<uint8_t*>(Ptr) - reinterpret_cast<uint8_t*>(Base));
    }
};
