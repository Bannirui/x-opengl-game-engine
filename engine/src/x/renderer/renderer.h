//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/renderer_api.h"

class Renderer
{
public:
    inline static RendererAPI::API GetAPI() { return s_rendererAPI; }

private:
    static RendererAPI::API s_rendererAPI;
};
