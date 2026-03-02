//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"
#include "x/core/base.h"

#include "x/renderer/renderer_api.h"

class Shader;
class OrthographicCamera;
class VertexArray;

class Renderer
{
public:
    static void Init();
    static void Shutdown();

    static void OnWindowResize(uint32_t width, uint32_t height);

    static void BeginScene(OrthographicCamera &camera);

    static void EndScene();

    /**
     * 绘制帧 VAO数据灌给GPU
     * @param vertexArray VAO
     */
    static void Submit(const X::Ref<Shader> &shader, const X::Ref<VertexArray> &vertexArray,
                       const glm::mat4 &transform = glm::mat4(1.0f));

    static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

private:
    struct SceneData
    {
        glm::mat4 viewProjectionMatrix;
    };

    static X::Scope<SceneData> s_sceneData;
};
