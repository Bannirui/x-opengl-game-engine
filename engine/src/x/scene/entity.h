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
        T& component = m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
        m_scene->onComponentAdded<T>(*this, component);
        return component;
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
        X_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        m_scene->m_registry.remove<T>(m_entityHandle);
    }

    // 转换
    operator bool() const { return m_entityHandle != entt::null; }
    operator entt::entity() const { return m_entityHandle; }
    operator uint32_t() const { return static_cast<uint32_t>(m_entityHandle); }
    // 比较
    bool operator==(const Entity& other) const
    {
        return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_entityHandle{entt::null};
    Scene*       m_scene{nullptr};
};
