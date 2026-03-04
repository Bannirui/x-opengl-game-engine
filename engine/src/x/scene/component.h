//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include "pch.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "x/scene/scene_camera.h"
#include "x/scene/scriptable_entity.h"

struct TagComponent
{
    std::string m_tag;

    TagComponent()                    = default;
    TagComponent(const TagComponent&) = default;
    TagComponent(const std::string& tag) : m_tag(tag) {}
};

struct TransformComponent
{
    glm::vec3 m_translation{0.0f};
    glm::vec3 m_rotation{0.0f};
    glm::vec3 m_scale{1.0f};

    TransformComponent()                          = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent(const glm::vec3& translation) : m_translation(translation) {}

    glm::mat4 GetTransform() const
    {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), m_rotation.x, {1, 0, 0}) *
                             glm::rotate(glm::mat4(1.0f), m_rotation.y, {0, 1, 0}) *
                             glm::rotate(glm::mat4(1.0f), m_rotation.z, {0, 0, 1});
        return glm::translate(glm::mat4(1.0f), m_translation) * rotation * glm::scale(glm::mat4(1.0f), m_scale);
    }
};

struct SpriteRendererComponent
{
    glm::vec4 m_color{1.0f};

    SpriteRendererComponent()                               = default;
    SpriteRendererComponent(const SpriteRendererComponent&) = default;
    SpriteRendererComponent(const glm::vec4& color) : m_color(color) {}
};

struct CameraComponent
{
    SceneCamera m_camera;
    bool        m_primary = true;
    bool        m_fixedAspectRatio{false};

    CameraComponent()                       = default;
    CameraComponent(const CameraComponent&) = default;
};

struct NativeScriptComponent
{
    ScriptableEntity* m_instance = nullptr;
    ScriptableEntity* (*m_instantiateScript)();
    void (*m_destroyScript)(NativeScriptComponent*);

    template <typename T>
    void Bind()
    {
        m_instantiateScript = []()
        {
            return static_cast<ScriptableEntity*>(new T());
        };
        m_destroyScript = [](NativeScriptComponent* nsc)
        {
            delete nsc->m_instance;
            nsc->m_instance = nullptr;
        };
    }
};