//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/renderer/camera.h"

class SceneCamera : public Camera
{
public:
    SceneCamera();
    ~SceneCamera() override;

    void SetOrthographic(float size, float nearClip, float farClip);
    void SetViewportSize(uint32_t width, uint32_t height);

    float get_orthographicSize() const { return m_orthographicSize; }
    void  SetOrthographicSize(float size)
    {
        m_orthographicSize = size;
        recalculateProjection();
    }

private:
    void recalculateProjection();

private:
    float m_orthographicSize{10.0f};
    float m_orthographicNear{-1.0f}, m_orthographicFar{1.0f};
    float m_aspectRatio{0.0f};
};
