//
// Created by dingrui on 3/3/26.
//

#pragma once

#include <glm/glm.hpp>

#include <x/core/layer.h>
#include <x/renderer/orthographic_camera_controller.h>
#include <x/scene/entity.h>

#include "panel/scene_hierarchy_panel.h"

class KeyPressEvent;
class Entity;
class Scene;
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
    void OnEvent(Event& e) override;

private:
    bool onKeyPressed(KeyPressEvent& e);

    void newScene();
    void openScene();
    void saveSceneAs();

private:
    OrthographicCameraController m_cameraController;

    X::Ref<VertexArray> m_squareVA;
    X::Ref<Shader>      m_flatColorShader;
    X::Ref<FrameBuffer> m_frameBuffer;

    X::Ref<Scene> m_activeScene;
    Entity        m_squareEntity;
    Entity        m_cameraEntity;
    Entity        m_secondCamera;

    X::Ref<Texture2D> m_checkerboardTexture;
    glm::vec2         m_viewportSize = {0.0f, 0.0f};
    bool              m_viewportFocused{false}, m_viewportHovered{false};
    glm::vec4         m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};

    // Panel
    SceneHierarchyPanel m_sceneHierarchyPanel;
};
