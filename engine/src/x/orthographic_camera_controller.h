//
// Created by rui ding on 2026/2/28.
//

#pragma once

#include "pch.h"

#include "x/renderer/orthographic_camera.h"

class Event;
class Timestep;
class WindowResizeEvent;
class MouseScrolledEvent;
class OrthographicCameraController
{
public:
    OrthographicCameraController(float aspectRatio, bool rotation = false);

    void OnUpdate(Timestep ts);
    void OnEvent(Event &e);

    OrthographicCamera       &get_camera() { return m_camera; }
    const OrthographicCamera &get_camera() const { return m_camera; }

private:
    bool onMouseScrolled(MouseScrolledEvent &e);
    bool onWindowResized(WindowResizeEvent &e);

private:
    float              m_aspectRatio;
    float              m_zoomLevel{1.0f};
    OrthographicCamera m_camera;
    bool               m_rotation;

    glm::vec3 m_cameraPosition;
    float     m_cameraRotation{0.0f};
    float     m_cameraTranslationSpeed{5.0f}, m_cameraRotationSpeed{180.0f};
};
