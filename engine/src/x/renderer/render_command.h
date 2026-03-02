//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include <glm/glm.hpp>

#include "x/core/base.h"
#include "x/renderer/renderer_api.h"

class VertexArray;

class RenderCommand
{
public:
    static void Init() { s_rendererAPI->Init(); }
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        s_rendererAPI->SetViewport(x, y, width, height);
    }

    static void SetClearColor(const glm::vec4 &color) { s_rendererAPI->SetClearColor(color); }

    static void Clear() { s_rendererAPI->Clear(); }

    static void DrawIndexed(const X::Ref<VertexArray> &vertexArray,uint32_t count = 0) { s_rendererAPI->DrawIndexed(vertexArray, count); }

private:
    static X::Scope<RendererAPI> s_rendererAPI;
};
