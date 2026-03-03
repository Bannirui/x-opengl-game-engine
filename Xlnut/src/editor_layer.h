//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/core/layer.h"
#include "x/renderer/orthographic_camera_controller.h"

#include <glm/glm.hpp>

class Texture2D;
class FrameBuffer;
class Shader;
class VertexArray;

class EditorLayer : public Layer
{
public:
    EditorLayer();
    ~EditorLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event &e) override;

private:
    OrthographicCameraController m_cameraController;

    X::Ref<VertexArray> m_squareVA;
    X::Ref<Shader>      m_flatColorShader;
    X::Ref<FrameBuffer> m_frameBuffer;
    X::Ref<Texture2D>   m_checkerboardTexture;
    glm::vec2           m_viewportSize = {0.0f, 0.0f};
    bool                m_viewportFocused{false}, m_viewportHovered{false};
    glm::vec4           m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};
};
