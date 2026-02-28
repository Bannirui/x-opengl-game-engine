//
// Created by rui ding on 2026/2/28.
//

#include "x/renderer/orthographic_camera_controller.h"

#include "x/core/input.h"
#include "x/core/key_codes.h"
#include "x/events/application_event.h"
#include "x/events/event.h"
#include "x/events/mouse_event.h"
#include "x/core/timestep.h"
#include "x/renderer/orthographic_camera.h"

OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
    : m_aspectRatio(aspectRatio),
      m_camera(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel),
      m_rotation(rotation)
{
}

void OrthographicCameraController::OnUpdate(Timestep ts)
{
    if (Input::IsKeyPressed(X_KEY::A))
    {
        m_cameraPosition.x -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
        m_cameraPosition.y -= sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
    }
    else if (Input::IsKeyPressed(X_KEY::D))
    {
        m_cameraPosition.x += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
        m_cameraPosition.y += sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
    }
    if (Input::IsKeyPressed(X_KEY::W))
    {
        m_cameraPosition.x += -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
        m_cameraPosition.y += cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
    }
    else if (Input::IsKeyPressed(X_KEY::S))
    {
        m_cameraPosition.x -= -sin(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
        m_cameraPosition.y -= cos(glm::radians(m_cameraRotation)) * m_cameraTranslationSpeed * ts;
    }

    if (m_rotation)
    {
        if (Input::IsKeyPressed(X_KEY::Q))
        {
            m_cameraRotation += m_cameraRotationSpeed * ts;
        }
        if (Input::IsKeyPressed(X_KEY::E))
        {
            m_cameraRotation -= m_cameraRotationSpeed * ts;
        }

        if (m_cameraRotation > 180.0f)
        {
            m_cameraRotation -= 360.0f;
        }
        else if (m_cameraRotation <= -180.0f)
        {
            m_cameraRotation += 360.0f;
        }
        m_camera.set_rotation(m_cameraRotation);
    }
    m_camera.set_position(m_cameraPosition);
    m_cameraTranslationSpeed = m_zoomLevel;
}

void OrthographicCameraController::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent &event) { return onMouseScrolled(event); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent &event) { return onWindowResized(event); });
}

bool OrthographicCameraController::onMouseScrolled(MouseScrolledEvent &e)
{
    m_zoomLevel -= e.get_yOffset() * 0.25f;
    m_zoomLevel = std::max(m_zoomLevel, 0.25f);
    m_camera.SetProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
    return false;
}

bool OrthographicCameraController::onWindowResized(WindowResizeEvent &e)
{
    m_aspectRatio = static_cast<float>(e.get_width()) / static_cast<float>(e.get_height());
    m_camera.SetProjection(-m_aspectRatio * m_zoomLevel, m_aspectRatio * m_zoomLevel, -m_zoomLevel, m_zoomLevel);
    return false;
}
