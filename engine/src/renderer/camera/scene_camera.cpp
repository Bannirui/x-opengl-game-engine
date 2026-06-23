//
// Created by dingrui on 3/3/26.
//

#include "x/renderer/camera/scene_camera.h"

#include "x/core/assert.h"

#include <glm/ext/matrix_clip_space.hpp>

SceneCamera::SceneCamera() {
    recalculateProjection();
}

SceneCamera::~SceneCamera() {}

void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip) {
    m_projectionType = ProjectionType::Perspective;
    m_perspectiveFOV = verticalFOV;
    m_perspectiveNear = nearClip;
    m_perspectiveFar = farClip;
    recalculateProjection();
}

void SceneCamera::SetOrthographic(float size, float nearClip, float farClip) {
    m_projectionType = ProjectionType::Orthographic;
    m_orthographicSize = size;
    m_orthographicNear = nearClip;
    m_orthographicFar = farClip;
    recalculateProjection();
}

void SceneCamera::SetViewportSize(uint32_t width, uint32_t height) {
    X_CORE_ASSERT(width > 0 && height > 0, "Invalid viewport size");
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    recalculateProjection();
}

void SceneCamera::recalculateProjection() {
    switch (m_projectionType) {
        case ProjectionType::Orthographic: {
            // 正交模式
            float orthoLeft = -m_orthographicSize * m_aspectRatio * 0.5f;
            float orthoRight = m_orthographicSize * m_aspectRatio * 0.5f;
            float orthoBottom = -m_orthographicSize * 0.5f;
            float orthoUp = m_orthographicSize * 0.5f;
            m_projection =
                glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoUp, m_orthographicNear, m_orthographicFar);
            break;
        }
        case ProjectionType::Perspective: {
            // 透视模式
            m_projection = glm::perspective(m_perspectiveFOV, m_aspectRatio, m_perspectiveNear, m_perspectiveFar);
            break;
        }
        default: {
            X_CORE_ERROR("Unsupported projection.");
            break;
        }
    }
}
