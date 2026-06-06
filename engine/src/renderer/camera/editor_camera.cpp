//
// Created by rui ding on 2026/3/6.
//

#include "x/renderer/camera/editor_camera.h"

#include "x/core/input.h"
#include "x/core/timestep.h"
#include "x/events/event.h"
#include "x/events/mouse_event.h"

#include <glm/gtx/quaternion.hpp>

EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
    : Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)), /* 投影矩阵 */
      m_fov(fov), /* 视角 角度 */
      m_aspectRatio(aspectRatio), /* 宽高比 */
      m_nearClip(nearClip), /* 近剪裁面 */
      m_farClip(farClip) /* 远剪裁面 */
{
    updateView();
}

EditorCamera::~EditorCamera() {}

void EditorCamera::OnUpdate(Timestep /* ts */) {
    // todo
    // if (Input::IsKeyPressed(X::KEY::LeftAlt)) {
        const glm::vec2& mouse{Input::GetMouseX(), Input::GetMouseY()};
        glm::vec2 delta = (mouse - m_lastMousePosition) * 0.003f;
        m_lastMousePosition = mouse;
        if (Input::IsMouseButtonPressed(X::MOUSE::ButtonMiddle)) {
            mousePan(delta);
        } else if (Input::IsMouseButtonPressed(X::MOUSE::ButtonLeft)) {
            mouseRotate(delta);
        } else if (Input::IsMouseButtonPressed(X::MOUSE::ButtonRight)) {
            mouseZoom(delta.y);
        }
    // }
    updateView();
}

void EditorCamera::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) {
        return onMouseScroll(e);
    });
}

glm::vec3 EditorCamera::GetUpDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 EditorCamera::GetRightDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 EditorCamera::GetForwardDirection() const {
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::quat EditorCamera::GetOrientation() const {
    return glm::quat(glm::vec3(-m_pitch, -m_yaw, 0.0f));
}

void EditorCamera::updateProjection() {
    m_aspectRatio = m_viewportWidth / m_viewportHeight;
    m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearClip, m_farClip);
}

void EditorCamera::updateView() {
    m_position = calculatePosition();
    glm::quat orientation = GetOrientation();
    m_viewMatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
    m_viewMatrix = glm::inverse(m_viewMatrix);
}

bool EditorCamera::onMouseScroll(MouseScrolledEvent& e) {
    float delta = e.get_yOffset() * 0.1f;
    mouseZoom(delta);
    updateView();
    return false;
}

void EditorCamera::mousePan(glm::vec2& delta) {
    auto [xSpeed, ySpeed] = panSpeed();
    m_focalPoint += -GetRightDirection() * delta.x * xSpeed * m_distance;
    m_focalPoint += GetUpDirection() * delta.y * ySpeed * m_distance;
}

void EditorCamera::mouseRotate(glm::vec2& delta) {
    float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
    m_yaw += yawSign * delta.x * rotationSpeed();
    m_pitch += delta.y * rotationSpeed();
}

void EditorCamera::mouseZoom(float /* delta */) {}

glm::vec3 EditorCamera::calculatePosition() const {
    return m_focalPoint - GetForwardDirection() * m_distance;
}

std::pair<float, float> EditorCamera::panSpeed() const {
    float x = std::min(m_viewportWidth / 1000.0f, 2.4f);  // max 2.4
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
    float y = std::min(m_viewportHeight / 1000.0f, 2.4f);  // max 2.4
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
    return {xFactor, yFactor};
}

float EditorCamera::rotationSpeed() const {
    return 0.8f;
}

float EditorCamera::zoomSpeed() const {
    float distance = m_distance * 0.2f;
    distance = std::max(distance, 0.0f);  // min 0
    float speed = distance * distance;
    speed = std::min(speed, 100.0f);  // max 100
    return speed;
}
