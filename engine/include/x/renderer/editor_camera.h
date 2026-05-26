//
// Created by rui ding on 2026/3/6.
//

#pragma once

#include "x/renderer/camera.h"

#include <glm/glm.hpp>

class MouseScrolledEvent;
class Event;
class Timestep;

class EditorCamera : public Camera
{
public:
    EditorCamera() = default;
    EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
    ~EditorCamera() override;

    void OnUpdate(Timestep ts);
    void OnEvent(Event& e);

    float get_distance() const { return m_distance; }

    void set_distance(float distance) { m_distance = distance; }

    void SetViewportSize(float width, float height)
    {
        m_viewportWidth  = width;
        m_viewportHeight = height;
        updateProjection();
    }

    const glm::mat4& get_viewMatrix() const { return m_viewMatrix; }

    glm::mat4 GetViewProjection() const { return m_projection * m_viewMatrix; }

    glm::vec3 GetUpDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetForwardDirection() const;

    const glm::vec3& get_position() const { return m_position; }

    glm::quat GetOrientation() const;

    float get_pitch() const { return m_pitch; }

    float get_yaw() const { return m_yaw; }

private:
    void updateProjection();
    void updateView();

    bool onMouseScroll(MouseScrolledEvent& e);

    void mousePan(glm::vec2& delta);
    void mouseRotate(glm::vec2& delta);
    void mouseZoom(float delta);

    glm::vec3 calculatePosition() const;

    std::pair<float, float> panSpeed() const;
    float                   rotationSpeed() const;
    float                   zoomSpeed() const;

private:
    float m_fov{45.0f}, m_aspectRatio{1.778f}, m_nearClip{0.1f}, m_farClip{1000.0f};

    glm::mat4 m_viewMatrix;
    glm::vec3 m_position{0.0f};
    glm::vec3 m_focalPoint{0.0f};

    glm::vec2 m_initialMousePosition{0.0f};

    float m_distance{10.0f};
    float m_pitch{0.0f}, m_yaw{0.0f};

    float m_viewportWidth{1280.0f}, m_viewportHeight{720.0f};
};
