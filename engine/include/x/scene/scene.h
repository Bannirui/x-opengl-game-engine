//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include "x/core/timestep.h"
#include "x/renderer/camera/editor_camera.h"

#include <entt/entt.hpp>

class UUID;
class Entity;
class b2World;

class Scene {
public:
    Scene();
    ~Scene();

    static Ref<Scene> Copy(Ref<Scene> other);

    Entity CreateEntity(const std::string& name = std::string());
    Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
    void DestroyEntity(Entity entity);

    entt::registry& Reg() {
        return m_registry;
    }

    void OnRuntimeStart();
    void OnRuntimeStop();
    void OnSimulationStart();
    void OnSimulationStop();
    void OnUpdateRuntime(Timestep ts);
    void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
    void OnUpdateEditor(Timestep ts, EditorCamera& camera);
    void OnViewportResize(uint32_t width, uint32_t height);

    entt::registry& get_registry() {
        return m_registry;
    }

    const entt::registry& get_registry() const {
        return m_registry;
    }

    void DuplicateEntity(Entity entity);
    Entity GetPrimaryCameraEntity();

    template <typename... Components>
    auto GetAllEntitiesWith() {
        return m_registry.view<Components...>();
    }

private:
    template <typename T>
    void onComponentAdded(Entity entity, T& component);

    void onPhysics2DStart();
    void onPhysics2DStop();

    void renderScene(EditorCamera& camera);

private:
    friend class Entity;
    friend class SceneSerializer;

    entt::registry m_registry;
    uint32_t m_viewportWidth{0}, m_viewportHeight{0};
    b2World* m_physicsWorld{nullptr};
};
