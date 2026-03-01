//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"
#include "x/core/core.h"

#include <glm/glm.hpp>

class VertexArray;

class RendererAPI
{
public:
    enum class API : uint8_t
    {
        kNone   = 0,
        kOpenGL = 1,
    };

public:
    virtual void Init()                                                               = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4 &color)                                = 0;
    virtual void Clear()                                                              = 0;

    virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) = 0;

    inline static API GetAPI() { return s_API; }

    static X::Scope<RendererAPI> Create();

private:
    static API s_API;
};
