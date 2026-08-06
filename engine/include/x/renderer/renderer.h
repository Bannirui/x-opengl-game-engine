//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/renderer_api.h"

#include <glm/glm.hpp>

#include <cstdint>

class Shader;
class OrthographicCamera;
class VertexArray;

class Renderer {
public:
    static void Init();
    static void Shutdown();

    /**
     * 重置窗口大小
     * @param width 新窗口的宽
     * @param height 新窗口的高
     */
    static void OnWindowResize(uint32_t width, uint32_t height);

    static RendererAPI::API GetAPI() {
        return RendererAPI::GetAPI();
    }

private:
    struct SceneData {
        glm::mat4 viewProjectionMatrix;
    };

    static Scope<SceneData> s_sceneData;
};
