//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/renderer_api.h"

class VertexArray;

class Renderer
{
public:
    static void BeginScene();
    static void EndScene();

    /**
     * 绘制帧 VAO数据灌给GPU
     * @param vertexArray VAO
     */
    static void Submit(const std::shared_ptr<VertexArray> &vertexArray);

    inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
};
