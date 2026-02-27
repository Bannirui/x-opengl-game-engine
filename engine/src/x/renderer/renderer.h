//
// Created by rui ding on 2026/2/27.
//

#pragma once

enum class RendererAPI : uint8_t
{
    kNone   = 0,
    kOpenGL = 1,
};

class Renderer
{
public:
    inline static RendererAPI GetAPI() { return s_rendererAPI; }

private:
    static RendererAPI s_rendererAPI;
};
