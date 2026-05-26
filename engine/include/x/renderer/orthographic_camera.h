//
// Created by dingrui on 2/28/26.
//

#pragma once

#include <glm/glm.hpp>

class OrthographicCamera {
public:
    OrthographicCamera(float left, float right, float bottom, float top);

    void SetProjection(float left, float right, float bottom, float top);

    const glm::vec3 &get_position() const { return m_position; }

    void set_position(const glm::vec3 &position) {
        m_position = position;
        reCalculateViewMatrix();
    }

    float get_rotation() const { return m_rotation; }

    void set_rotation(float rotation) {
        m_rotation = rotation;
        reCalculateViewMatrix();
    }

    const glm::mat4 &get_projectionMatrix() const { return m_projectionMatrix; }
    const glm::mat4 &get_viewMatrix() const { return m_viewMatrix; }
    const glm::mat4 &get_viewProjectionMatrix() const { return m_viewProjectionMatrix; }

private:
    void reCalculateViewMatrix();

private:
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_viewProjectionMatrix;

    glm::vec3 m_position{0.0f, 0.0f, 0.0f};
    float m_rotation{0.0f};
};
