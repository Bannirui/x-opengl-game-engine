//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"

#include "glm/glm.hpp"

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
    virtual void SetClearColor(const glm::vec4 &color) = 0;
    virtual void Clear()                               = 0;

    virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) = 0;

    inline static API GetAPI() { return s_API; }

private:
    static API s_API;
};
