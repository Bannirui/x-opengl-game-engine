//
// Created by rui ding on 2026/3/3.
//

#include "x/scene/scene.h"

#include "x/scene/component.h"
#include "x/scene/entity.h"
#include "x/core/timestep.h"
#include "x/renderer/renderer_2D.h"

#include <glm/glm.hpp>

Scene::Scene() {}

Scene::~Scene() {}

Entity Scene::CreateEntity(const std::string& name)
{
    Entity entity = {m_registry.create(), this};
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>();
    tag.m_tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_registry.destroy(entity);
}

void Scene::OnUpdateRuntime(Timestep ts)
{
    // Update scripts
    {
        m_registry.view<NativeScriptComponent>().each(
            [=](auto entity, auto& nsc)
            {
                if (!nsc.m_instance)
                {
                    nsc.m_instance           = nsc.m_instantiateScript();
                    nsc.m_instance->m_entity = Entity(entity, this);
                    nsc.m_instance->OnCreate();
                }
                nsc.m_instance->OnUpdate(ts);
            });
    }
    // Render 2D
    Camera*   mainCamera = nullptr;
    glm::mat4 cameraTransform;
    {
        auto view = m_registry.view<TransformComponent, CameraComponent>();
        for (auto entity : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
            if (camera.m_primary)
            {
                mainCamera      = &camera.m_camera;
                cameraTransform = transform.GetTransform();
                break;
            }
        }
    }
    if (mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera, cameraTransform);
        auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
        }
        Renderer2D::EndScene();
    }
}

void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
{
    Renderer2D::BeginScene(camera);
    auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group)
    {
        auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
        Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
    }
    Renderer2D::EndScene();
}

void Scene::OnViewportResize(uint32_t width, uint32_t height)
{
    m_viewportWidth  = width;
    m_viewportHeight = height;

    auto view = m_registry.view<CameraComponent>();
    for (auto entity : view)
    {
        auto& cameraComponent = view.get<CameraComponent>(entity);
        if (!cameraComponent.m_fixedAspectRatio)
        {
            cameraComponent.m_camera.SetViewportSize(width, height);
        }
    }
}

Entity Scene::GetPrimaryCameraEntity()
{
    auto view = m_registry.view<CameraComponent>();
    for (auto entity : view)
    {
        const auto& camera = view.get<CameraComponent>(entity);
        if (camera.m_primary)
        {
            return Entity{entity, this};
        }
    }
    return {};
}

template <typename T>
void Scene::onComponentAdded(Entity entity, T& component)
{
    X_ASSERT(false, "Not supported");
}

template <>
void Scene::onComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
{
}

template <>
void Scene::onComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
{
    component.m_camera.SetViewportSize(m_viewportWidth, m_viewportHeight);
}

template <>
void Scene::onComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
{
}

template <>
void Scene::onComponentAdded<TagComponent>(Entity entity, TagComponent& component)
{
}

template <>
void Scene::onComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
{
}