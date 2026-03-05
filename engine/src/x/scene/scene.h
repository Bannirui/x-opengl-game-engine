//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <entt/entt.hpp>

#include "x/core/timestep.h"

class Entity;

class Scene
{
public:
    Scene();
    ~Scene();

    Entity          CreateEntity(const std::string& name = std::string());
    void            DestroyEntity(Entity entity);
    entt::registry& Reg() { return m_registry; }

    void OnUpdate(Timestep ts);
    void OnViewportResize(uint32_t width, uint32_t height);

    entt::registry&       get_registry() { return m_registry; }
    const entt::registry& get_registry() const { return m_registry; }

    Entity GetPrimaryCameraEntity();

private:
    template <typename T>
    void onComponentAdded(Entity entity, T& component);

private:
    entt::registry m_registry;
    uint32_t       m_viewportWidth{0}, m_viewportHeight{0};

    friend class Entity;
    friend class SceneSerializer;
};
