//
// Created by rui ding on 2026/3/3.
//

#include "x/scene/scene.h"

#include "x/scene/component.h"
#include "x/scene/entity.h"
#include "x/core/timestep.h"
#include "x/renderer/renderer_2D.h"

#include <glm/glm.hpp>

// static void doMath(const glm::mat4& transform) {}
// static void onTransformConstruct(entt::registry& registry, entt::entity entity) {}

Scene::Scene()
{
#if ENTT_EXAMPLE_CODE
    entt::entity entity = m_registry.create();
    m_registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
    m_registry.on_construct<TransformComponent>().connect<&onTransformConstruct>();
    if (m_registry.any_of<TransformComponent>(entity))
    {
        TransformComponent& transform = m_registry.get<TransformComponent>(entity);
    }
    auto view = m_registry.view<TransformComponent>();
    for (auto entity : view)
    {
        TransformComponent& transform = view.get<TransformComponent>(entity);
    }

    auto group = m_registry.group<TransformComponent>(entt::get<MeshComponent>);
    for (auto entity : group)
    {
        auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
    }
#endif
}

Scene::~Scene() {}

Entity Scene::CreateEntity(const std::string& name)
{
    Entity entity = {m_registry.create(), this};
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>();
    tag.m_tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::OnUpdate(Timestep ts)
{
    auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group)
    {
        auto& transform = group.get<TransformComponent>(entity);
        auto& sprite    = group.get<SpriteRendererComponent>(entity);
        Renderer2D::DrawQuad(transform, sprite.m_color);
    }
}
