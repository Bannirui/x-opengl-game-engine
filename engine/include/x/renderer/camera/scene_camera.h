//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/renderer/camera/camera.h"

/**
 * 透视/正交双模
 * 只管投影 不持有位置和朝向 作为ECS CameraComponent的成员 view矩阵由实体的TransformComponent外部提供
 * 这样一份相机配置可以序列化 运行时挂到任意实体上
 */
class SceneCamera : public Camera {
public:
    enum class ProjectionType {
        Perspective = 1,   // 透视投影
        Orthographic = 2,  // 正交投影
    };

public:
    SceneCamera();
    ~SceneCamera() override;

    ProjectionType get_projectionType() const {
        return m_projectionType;
    }

    void set_projectionType(ProjectionType type) {
        m_projectionType = type;
    }

    /**
     * 切换到透视模式
     * @param verticalFOV
     * @param nearClip
     * @param farClip
     */
    void SetPerspective(float verticalFOV, float nearClip, float farClip);
    /**
     * 切换到正交模式
     * @param size
     * @param nearClip
     * @param farClip
     */
    void SetOrthographic(float size, float nearClip, float farClip);

    /**
     * 设宽高比
     * @param width
     * @param height
     */
    void SetViewportSize(uint32_t width, uint32_t height);

    float get_perspectiveFOV() const {
        return m_perspectiveFOV;
    }

    void SetPerspectiveFOV(float verticalFOV) {
        m_perspectiveFOV = verticalFOV;
        recalculateProjection();
    }

    float get_perspectiveNearClip() const {
        return m_perspectiveNear;
    }

    void SetPerspectiveNearClip(float nearClip) {
        m_perspectiveNear = nearClip;
        recalculateProjection();
    }

    float get_perspectiveFarClip() const {
        return m_perspectiveFar;
    }

    void SetPerspectiveFarClip(float farClip) {
        m_perspectiveFar = farClip;
        recalculateProjection();
    }

    float get_orthographicSize() const {
        return m_orthographicSize;
    }

    void SetOrthographicSize(float size) {
        m_orthographicSize = size;
        recalculateProjection();
    }

    float get_orthographicNearClip() const {
        return m_orthographicNear;
    }

    void SetOrthographicNearClip(float nearClip) {
        m_orthographicNear = nearClip;
        recalculateProjection();
    }

    float get_orthographicFarClip() const {
        return m_orthographicFar;
    }

    void SetOrthographicFarClip(float farClip) {
        m_orthographicFar = farClip;
        recalculateProjection();
    }

private:
    /**
     * 根据当前模式 重算投影矩阵
     */
    void recalculateProjection();

private:
    // 投影模式
    ProjectionType m_projectionType = ProjectionType::Orthographic;
    // 透视FOV弧度
    float m_perspectiveFOV{glm::radians(45.0f)};
    // 透视近平面 远平面
    float m_perspectiveNear{0.01f}, m_perspectiveFar{1000.0f};
    // 正交相机高度的一半
    float m_orthographicSize{10.0f};
    // 正交近平面 正交远平面
    float m_orthographicNear{-1.0f}, m_orthographicFar{1.0f};
    // 宽高比
    float m_aspectRatio{0.0f};
};
