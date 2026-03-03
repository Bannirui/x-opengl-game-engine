//
// Created by dingrui on 3/3/26.
//

#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera() = default;
    Camera(glm::mat4& projection) : m_projection(projection) {}
    virtual ~Camera() = default;

    const glm::mat4& get_projection() const { return m_projection; }

protected:
    glm::mat4 m_projection{glm::mat4(1.0f)};
};
