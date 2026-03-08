//
// Created by dingrui on 3/3/26.
//

#pragma once

#include <glm/glm.hpp>
#include <x/core/layer.h>
#include <x/renderer/editor_camera.h>
#include <x/renderer/orthographic_camera_controller.h>
#include <x/scene/entity.h>

#include "panel/content_browser_panel.h"
#include "panel/scene_hierarchy_panel.h"

class MouseButtonPressedEvent;
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
    bool onMouseButtonPressed(MouseButtonPressedEvent& e);

    void newScene();
    void openScene();
    void openScene(const std::filesystem::path& path);
    void saveSceneAs();

private:
    OrthographicCameraController m_cameraController;

    X::Ref<VertexArray> m_squareVA;
    X::Ref<Shader>      m_flatColorShader;
    X::Ref<FrameBuffer> m_framebuffer;

    X::Ref<Scene> m_activeScene;
    Entity        m_squareEntity;
    Entity        m_cameraEntity;
    Entity        m_secondCamera;
    Entity        m_hoveredEntity;

    EditorCamera m_editorCamera;

    X::Ref<Texture2D> m_checkerboardTexture;
    glm::vec2         m_viewportSize = {0.0f, 0.0f};
    glm::vec2         m_viewportBounds[2];
    bool              m_viewportFocused{false}, m_viewportHovered{false};
    glm::vec4         m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};

    int m_gizmoType{-1};
    // Panel
    SceneHierarchyPanel m_sceneHierarchyPanel;
    ContentBrowserPanel m_contentBrowserPanel;
};
