//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <cstdint>

class VertexArray;

class RendererAPI {
public:
    enum class API : uint8_t {
        kNone = 0,
        kOpenGL = 1,
    };

protected:
    RendererAPI() = default;

public:
    virtual ~RendererAPI() = default;

    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;

    /**
     * @param vertexArray VAO在内存的什么位置
     * @param indexCount VAO管理了多少个VBO顶点
     */
    virtual void DrawIndexed(const X::Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;
    virtual void DrawLines(const X::Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

    virtual void SetLineWidth(float width) = 0;

    static API GetAPI() {
        return s_API;
    }

    static X::Scope<RendererAPI> Create();

private:
    static API s_API;
};
