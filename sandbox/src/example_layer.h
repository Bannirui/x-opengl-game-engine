//
// Created by dingrui on 3/2/26.
//

#pragma once

#include "x/core/layer.h"
#include "x/core/core.h"
#include "x/renderer/orthographic_camera_controller.h"
#include "x/renderer/shader.h"

#include <glm/glm.hpp>

class OrthographicCameraController;
class Texture;
class VertexArray;
class Shader;
class ShaderLib;

class ExampleLayer : public Layer
{
public:
    ExampleLayer();
    ~ExampleLayer() override;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;

    void OnEvent(Event &e) override;

private:
    ShaderLib           m_shaderLib;
    X::Ref<Shader>      m_shader;       // shader
    X::Ref<VertexArray> m_vertexArray;  // VAO

    X::Ref<Shader>      m_flatColorShader;
    X::Ref<VertexArray> m_squareVAO;

    X::Ref<Texture> m_texture, m_logoTexture;

    OrthographicCameraController m_cameraController;

    glm::vec3 m_color{0.2f, 0.3f, 0.8f};
};
