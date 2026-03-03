//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include "pch.h"
#include "x/core/timestep.h"

#include <glm/glm.hpp>

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
    glm::mat4 m_transform{1.0f};

    TransformComponent()                          = default;
    TransformComponent(const TransformComponent&) = default;
    TransformComponent(const glm::mat4& transform) : m_transform(transform) {}

    operator glm::mat4&() { return m_transform; }
    operator const glm::mat4&() const { return m_transform; }
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

    std::function<void()> m_instantiateFunction;
    std::function<void()> m_destroyInstanceFunction;

    std::function<void(ScriptableEntity*)>           m_onCreateFunction;
    std::function<void(ScriptableEntity*)>           m_onDestroyFunction;
    std::function<void(ScriptableEntity*, Timestep)> m_onUpdateFunction;

    template <typename T>
    void Bind()
    {
        m_instantiateFunction     = [&]() { m_instance = new T(); };
        m_destroyInstanceFunction = [&]()
        {
            delete (T*)m_instance;
            m_instance = nullptr;
        };
        m_onCreateFunction  = [&](ScriptableEntity* instance) { ((T*)instance)->OnCreate(); };
        m_onDestroyFunction = [&](ScriptableEntity* instance) { ((T*)instance)->OnDestroy(); };
        m_onUpdateFunction  = [&](ScriptableEntity* instance, Timestep ts) { ((T*)instance)->OnUpdate(ts); };
    }
};