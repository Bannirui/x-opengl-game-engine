//
// Created by rui ding on 2026/3/6.
//

#pragma once

#include "x/renderer/camera/camera.h"

#include <glm/glm.hpp>

class MouseScrolledEvent;
class Event;
class Timestep;

/**
 * 透视轨迹球相机 编辑器用 自持view
 *   - 鼠标左键->绕焦点旋转
 *   - 鼠标中键->平移焦点
 *   - 鼠标滚轮/鼠标右键->缩放距离
 * 全部的操作需要按住键盘左Alt
 * 是自包含的完整相机 自己管理投影+视图+输入
 */
class EditorCamera : public Camera {
public:
    EditorCamera() = default;
    /**
     * @param fov 视角 角度
     * @param aspectRatio 宽高比
     * @param nearClip 近剪裁面
     * @param farClip 远剪裁面
     */
    EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);
    ~EditorCamera() override;

    // 每一帧都会调用过来
    void OnUpdate(Timestep ts);
    void OnEvent(Event& e);

    float get_distance() const {
        return m_distance;
    }

    /**
     * @param distance 相机到焦点的距离
     */
    void set_distance(float distance) {
        m_distance = distance;
    }

    void SetViewportSize(float width, float height) {
        m_viewportWidth = width;
        m_viewportHeight = height;
        updateProjection();
    }

    const glm::mat4& get_viewMatrix() const {
        return m_viewMatrix;
    }

    /**
     * MVP变换
     * 剪裁空间坐标
     * gl_Position = 投影矩阵*观察矩阵*模型矩阵*本地坐标
     * @return 投影矩阵*观察矩阵
     */
    glm::mat4 GetViewProjection() const {
        return m_projection * m_viewMatrix;
    }

    glm::vec3 GetUpDirection() const;
    glm::vec3 GetRightDirection() const;
    glm::vec3 GetForwardDirection() const;

    const glm::vec3& get_position() const {
        return m_position;
    }

    glm::quat GetOrientation() const;

    float get_pitch() const {
        return m_pitch;
    }

    float get_yaw() const {
        return m_yaw;
    }

private:
    void updateProjection();
    void updateView();

    bool onMouseScroll(MouseScrolledEvent& e);

    void mousePan(glm::vec2& delta);
    void mouseRotate(glm::vec2& delta);
    void mouseZoom(float delta);

    glm::vec3 calculatePosition() const;

    std::pair<float, float> panSpeed() const;
    float rotationSpeed() const;
    float zoomSpeed() const;

private:
    // 垂直FOV角度 宽高比 近平面 远平面
    float m_fov{45.0f}, m_aspectRatio{1.778f}, m_nearClip{0.1f}, m_farClip{1000.0f};
    // 视图矩阵 由pitch/yaw/focalPoint/distance计算出来的
    glm::mat4 m_viewMatrix;
    // 相机的世界坐标 默认相机就在世界坐标原点
    glm::vec3 m_position{0.0f};
    // 轨道球的焦点 相机围绕它旋转
    glm::vec3 m_focalPoint{0.0f};
    // 上一帧鼠标位置 用来计算delta的
    glm::vec2 m_lastMousePosition{0.0f};

    // 相机到焦点的距离
    float m_distance{10.0f};
    // 俯仰角-绕右轴旋转 偏航角-绕世界Y轴旋转
    float m_pitch{0.0f}, m_yaw{0.0f};
    // 视窗宽度 视口高度
    float m_viewportWidth{1280.0f}, m_viewportHeight{720.0f};
};
