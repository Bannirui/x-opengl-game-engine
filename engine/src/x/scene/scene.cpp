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

void Scene::OnUpdate(Timestep ts)
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
    Camera*    mainCamera      = nullptr;
    glm::mat4* cameraTransform = nullptr;
    {
        auto view = m_registry.view<TransformComponent, CameraComponent>();
        for (auto entity : view)
        {
            auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
            if (camera.m_primary)
            {
                mainCamera      = &camera.m_camera;
                cameraTransform = &transform.m_transform;
                break;
            }
        }
    }
    if (mainCamera)
    {
        Renderer2D::BeginScene(*mainCamera, *cameraTransform);
        auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawQuad(transform, sprite.m_color);
        }
        Renderer2D::EndScene();
    }
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
