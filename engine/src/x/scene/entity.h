//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <entt/entt.hpp>

#include "x/scene/scene.h"

class Scene;

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene);
    Entity(const Entity& other) = default;

    template <typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        X_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
        return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
    }

    template <typename T>
    T& GetComponent()
    {
        X_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return m_scene->m_registry.get<T>(m_entityHandle);
    }

    template <typename T>
    bool HasComponent()
    {
        return m_scene->m_registry.any_of<T>(m_entityHandle);
    }

    template <typename T>
    void RemoveComponent()
    {
        HZ_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        m_scene->m_registry.remove<T>(m_entityHandle);
    }

    operator bool() const { return m_entityHandle != entt::null; }

private:
    entt::entity m_entityHandle{entt::null};
    Scene*       m_scene{nullptr};
};
