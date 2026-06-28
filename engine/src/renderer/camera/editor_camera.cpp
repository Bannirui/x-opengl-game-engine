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
    // 计算出最新 当前的ViewMatrix
    updateView();
}

EditorCamera::~EditorCamera() {}

void EditorCamera::OnUpdate(Timestep ts) {
    /**
     * 鼠标当前帧位置
     * 屏幕的坐标是X向右是增长 Y向下是增长
     *   - 鼠标向右滑动时 delta.x>0
     *   - 鼠标向下滑动时 delta.y>0
     */
    const glm::vec2 mouse{Input::GetMouseX(), Input::GetMouseY()};
    // 鼠标灵敏度 计算出鼠标当前帧位置和上一帧位置的偏移量
    glm::vec2 delta = (mouse - m_lastMousePosition) * 0.003f;
    m_lastMousePosition = mouse;

    if (Input::IsKeyPressed(KeyCode::LeftAlt)) {
        float dt = ts.GetSeconds();
        if (Input::IsMouseButtonPressed(MouseCode::ButtonMiddle)) {
            // alt+鼠标中键 拖动
            mousePan(delta);
        } else if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) {
            // alt+鼠标左键 旋转
            mouseRotate(delta);
        } else if (Input::IsMouseButtonPressed(MouseCode::ButtonRight)) {
            mouseZoom(delta.y * dt * 10.0f);
        }
        // 发生了鼠标移动操作可能导致ViewMatrix的更新
        updateView();
    }
}

void EditorCamera::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    // 如果是鼠标滚轮滚动就要处理
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
    return glm::quat(glm::vec3(-m_pitch, /**/
                                -m_yaw, /**/
                                0.0f));
}

void EditorCamera::SetLookDirection(const glm::vec3& direction) {
    glm::vec3 forward = glm::normalize(direction);
    glm::quat rotation = glm::rotation(glm::vec3(0.0f, 0.0f, -1.0f), forward);
    glm::vec3 euler = glm::eulerAngles(rotation);
    m_pitch = -euler.x;
    m_yaw = -euler.y;
    updateView();
}

void EditorCamera::updateProjection() {
    // 宽高比
    m_aspectRatio = m_viewportWidth / m_viewportHeight;
    // 投影矩阵的公式
    m_projection = glm::perspective(glm::radians(m_fov), /* 角度->弧度 */
                                    m_aspectRatio, /* 宽高比 */
                                    m_nearClip, /* 近距离 */
                                    m_farClip /* 远距离 */
    );
}

void EditorCamera::updateView() {
    // 相机位置
    m_position = calculatePosition();
    // 相机的四元数
    glm::quat orientation = GetOrientation();
    // 先把四元数转换成旋转矩阵 再构造出camera transform
    m_viewMatrix = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
    // 为什么要求逆 ViewMatrix的本质是把世界变换到相机空间 不是相机如何移动 而是世界如何反向移动
    m_viewMatrix = glm::inverse(m_viewMatrix);
}

bool EditorCamera::onMouseScroll(MouseScrolledEvent& e) {
    // 拿到鼠标滚轮前后滚动的偏移 用灵敏度处理一下
    float delta = e.get_yOffset() * 0.1f;
    mouseZoom(delta);
    updateView();
    return false;
}

void EditorCamera::mousePan(glm::vec2& delta) {
    // 拿到当前视口大小下鼠标拖动的速度
    auto [xSpeed, ySpeed] = panSpeed();
    // 屏幕坐标X是向右增长的 鼠标向右拖动时delta.x>0 焦点往需要往左
    m_focalPoint += -GetRightDirection() * delta.x * xSpeed * m_distance;
    // 屏幕坐标Y是向下增长的 鼠标向下拖动时delta.y>0 焦点需要往上
    m_focalPoint += GetUpDirection() * delta.y * ySpeed * m_distance;
}

void EditorCamera::mouseRotate(glm::vec2& delta) {
    float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
    // x的偏移量加到偏航角
    m_yaw += yawSign * delta.x * rotationSpeed();
    // y的偏移量加到俯仰角
    m_pitch += delta.y * rotationSpeed();
}

void EditorCamera::mouseZoom(float delta) {
    /**
     * 滚轮前后滚动偏移
     *   - 滚轮向上 >0 距离变小 放大效果
     *   - 滚轮向下 <0 距离变大 缩小效果
     */
    m_distance -= delta * zoomSpeed();
    // 缩小有下界保护
    m_distance = std::max(m_distance, 0.1f);
}

glm::vec3 EditorCamera::calculatePosition() const {
    // 从焦点出发 沿着相机视线的把方向走distance就到了相机
    return m_focalPoint - GetForwardDirection() * m_distance;
}

std::pair<float, float> EditorCamera::panSpeed() const {
    // 根据视口宽度计算平移速度 把视口宽度归一化到[0,2.4]范围
    float x = std::min(m_viewportWidth / 1000.0f, 2.4f);  // max 2.4
    // 这个二次函数开口向上 顶点在x=2.43 所以整个区间[0.2.4]是在单调递减 也就是随着视口的增大 鼠标的平移速度在下降
    // 目的是让鼠标拖动平移的手感在不同视口大小下保持一致
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
    float y = std::min(m_viewportHeight / 1000.0f, 2.4f);  // max 2.4
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
    return {xFactor, yFactor};
}

float EditorCamera::rotationSpeed() const {
    return 0.8f;
}

float EditorCamera::zoomSpeed() const {
    /**
     * 核心思想 离焦点越远 缩放越快
     * 这样近距离能精确调整 远距离能快速拉近拉远
     */
    // 缩放
    float distance = m_distance * 0.2f;
    // 下界保护
    distance = std::max(distance, 0.0f);  // min 0
    // 平方
    float speed = distance * distance;
    // 上界限制
    speed = std::min(speed, 100.0f);  // max 100
    return speed;
}
