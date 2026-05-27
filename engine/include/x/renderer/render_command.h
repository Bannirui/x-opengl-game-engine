//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/renderer_api.h"

#include <glm/glm.hpp>

class VertexArray;

class RenderCommand {
public:
    static void Init() {
        s_rendererAPI->Init();
    }

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        s_rendererAPI->SetViewport(x, y, width, height);
    }

    static void SetClearColor(const glm::vec4& color) {
        s_rendererAPI->SetClearColor(color);
    }

    static void Clear() {
        s_rendererAPI->Clear();
    }

    /**
     * 调用底层绘制API DrawElements
     * 用索引方式告诉GPU怎么选取这些顶点
     * @param vertexArray VAO数据在CPU侧内存什么位置
     * @param count VAO管理了多少个顶点VBO
     */
    static void DrawIndexed(const X::Ref<VertexArray>& vertexArray, uint32_t count = 0) {
        s_rendererAPI->DrawIndexed(vertexArray, count);
    }

    /**
     * 底层绘制线段API DrawArrays
     * @param vertexArray VBO数据在哪儿
     * @param vertexCount 要画多少个顶点
     */
    static void DrawLines(const X::Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
        s_rendererAPI->DrawLines(vertexArray, vertexCount);
    }

    static void SetLineWidth(float width) {
        s_rendererAPI->SetLineWidth(width);
    }

private:
    static X::Scope<RendererAPI> s_rendererAPI;
};
