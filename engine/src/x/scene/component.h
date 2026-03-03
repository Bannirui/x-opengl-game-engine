//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <glm/glm.hpp>

struct TransformComponent
{
    glm::mat4 m_transform{1.0f};

    TransformComponent()                           = default;
    TransformComponent(const TransformComponent &) = default;
    TransformComponent(const glm::mat4 &transform) : m_transform(transform) {}

    operator glm::mat4 &() { return m_transform; }
    operator const glm::mat4 &() const { return m_transform; }
};

struct SpriteRendererComponent
{
    glm::vec4 m_color{1.0f};

    SpriteRendererComponent()                                = default;
    SpriteRendererComponent(const SpriteRendererComponent &) = default;
    SpriteRendererComponent(const glm::vec4 &color) : m_color(color) {}
};