//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <entt/entt.hpp>

class Timestep;

class Scene
{
public:
    Scene();
    ~Scene();

    entt::entity    CreateEntity();
    entt::registry &Reg() { return m_registry; }

    void OnUpdate(Timestep ts);

private:
    entt::registry m_registry;
};
