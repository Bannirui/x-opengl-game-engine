//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <entt/entt.hpp>

class Timestep;
class Entity;

class Scene
{
public:
    Scene();
    ~Scene();

    Entity          CreateEntity(const std::string& name = std::string());
    entt::registry& Reg() { return m_registry; }

    void OnUpdate(Timestep ts);

private:
    entt::registry m_registry;
    friend class Entity;
};
