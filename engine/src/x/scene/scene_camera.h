//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/renderer/camera.h"

class SceneCamera : public Camera
{
public:
    enum class ProjectionType
    {
        Perspective  = 0,
        Orthographic = 1,
    };

public:
    SceneCamera();
    ~SceneCamera() override;

    ProjectionType get_projectionType() const { return m_projectionType; }
    void           set_projectionType(ProjectionType type) { m_projectionType = type; }

    void SetPerspective(float verticalFOV, float nearClip, float farClip);
    void SetOrthographic(float size, float nearClip, float farClip);

    void SetViewportSize(uint32_t width, uint32_t height);

    float get_perspectiveFOV() const { return m_perspectiveFOV; }
    void  SetPerspectiveFOV(float verticalFOV)
    {
        m_perspectiveFOV = verticalFOV;
        recalculateProjection();
    }
    float get_perspectiveNearClip() const { return m_perspectiveNear; }
    void  SetPerspectiveNearClip(float nearClip)
    {
        m_perspectiveNear = nearClip;
        recalculateProjection();
    }
    float get_perspectiveFarClip() const { return m_perspectiveFar; }
    void  SetPerspectiveFarClip(float farClip)
    {
        m_perspectiveFar = farClip;
        recalculateProjection();
    }

    float get_orthographicSize() const { return m_orthographicSize; }
    void  SetOrthographicSize(float size)
    {
        m_orthographicSize = size;
        recalculateProjection();
    }
    float get_orthographicNearClip() const { return m_orthographicNear; }
    void  SetOrthographicNearClip(float nearClip)
    {
        m_orthographicNear = nearClip;
        recalculateProjection();
    }
    float get_orthographicFarClip() const { return m_orthographicFar; }
    void  SetOrthographicFarClip(float farClip)
    {
        m_orthographicFar = farClip;
        recalculateProjection();
    }

private:
    void recalculateProjection();

private:
    ProjectionType m_projectionType = ProjectionType::Orthographic;

    float m_perspectiveFOV{glm::radians(45.0f)};
    float m_perspectiveNear{0.01f}, m_perspectiveFar{1000.0f};

    float m_orthographicSize{10.0f};
    float m_orthographicNear{-1.0f}, m_orthographicFar{1.0f};

    float m_aspectRatio{0.0f};
};
