//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/renderer_api.h"

class OpenGLRendererAPI : public RendererAPI
{
public:
    void Init() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void SetClearColor(const glm::vec4 &color) override;
    void Clear() override;
    void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray) override;
};
