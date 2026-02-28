//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/renderer/orthographic_camera.h"

class Renderer2D {
public:
    static void Init();

    static void Shutdown();

    static void BeginScene(const OrthographicCamera &camera);

    static void EndScene();

    static void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);

    static void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
};
