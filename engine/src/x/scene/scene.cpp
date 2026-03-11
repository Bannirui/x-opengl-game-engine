//
// Created by rui ding on 2026/3/3.
//

#include "x/scene/scene.h"

#include <glm/glm.hpp>

#include <box2d/box2d.h>

#include "x/core/timestep.h"
#include "x/renderer/renderer_2D.h"
#include "x/scene/component.h"
#include "x/scene/entity.h"

static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
{
    switch (bodyType)
    {
        case Rigidbody2DComponent::BodyType::Static:
        {
            return b2_staticBody;
        }
        case Rigidbody2DComponent::BodyType::Dynamic:
        {
            return b2_dynamicBody;
        }
        case Rigidbody2DComponent::BodyType::Kinematic:
        {
            return b2_kinematicBody;
        }
        default:
            break;
    }
    X_CORE_ASSERT(false, "Unknown body type");
    return b2_staticBody;
}

template <typename Component>
static void CopyComponent(entt::registry& dst, entt::registry& src,
                          const std::unordered_map<UUID, entt::entity>& enttMap)
{
    auto view = src.view<Component>();
    for (auto e : view)
    {
        UUID uuid = src.get<IDComponent>(e).id;
        X_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
        entt::entity dstEnttID = enttMap.at(uuid);

        auto& component = src.get<Component>(e);
        dst.emplace_or_replace<Component>(dstEnttID, component);
    }
}

template <typename Component>
static void CopyComponentIfExists(Entity dst, Entity src)
{
    if (src.HasComponent<Component>()) dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
}

Scene::Scene() {}

Scene::~Scene() {}

X::Ref<Scene> Scene::Copy(X::Ref<Scene> other)
{
    X::Ref<Scene> newScene                                  = X::CreateRef<Scene>();
    newScene->m_viewportWidth                               = other->m_viewportWidth;
    newScene->m_viewportHeight                              = other->m_viewportHeight;
    auto&                                  srcSceneRegistry = other->m_registry;
    auto&                                  dstSceneRegistry = newScene->m_registry;
    std::unordered_map<UUID, entt::entity> enttMap;

    // Create entities in new scene
    auto idView = srcSceneRegistry.view<IDComponent>();
    for (auto e : idView)
    {
        UUID        uuid      = srcSceneRegistry.get<IDComponent>(e).id;
        const auto& name      = srcSceneRegistry.get<TagComponent>(e).m_tag;
        Entity      newEntity = newScene->CreateEntityWithUUID(uuid, name);
        enttMap[uuid]         = (entt::entity)newEntity;
    }
    // Copy components (except IDComponent and TagComponent)
    CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    return newScene;
}

Entity Scene::CreateEntity(const std::string& name)
{
    return CreateEntityWithUUID(UUID(), name);
}

Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
{
    Entity entity = {m_registry.create(), this};
    entity.AddComponent<IDComponent>();
    entity.AddComponent<TransformComponent>();
    auto& tag = entity.AddComponent<TagComponent>();
    tag.m_tag = name.empty() ? "Entity" : name;
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_registry.destroy(entity);
}

void Scene::OnRuntimeStart()
{
    m_physicsWorld = new b2World({0.0f, -9.8f});

    auto view = m_registry.view<Rigidbody2DComponent>();
    for (auto e : view)
    {
        Entity entity    = {e, this};
        auto&  transform = entity.GetComponent<TransformComponent>();
        auto&  rb2d      = entity.GetComponent<Rigidbody2DComponent>();
        // Body
        b2BodyDef bodyDef;
        bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.m_translation.x, transform.m_translation.y);
        bodyDef.angle = transform.m_rotation.z;

        b2Body* body = m_physicsWorld->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);
        rb2d.RuntimeBody = body;

        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            auto&          bc2d = entity.GetComponent<BoxCollider2DComponent>();
            b2PolygonShape boxShape;
            boxShape.SetAsBox(bc2d.Size.x * transform.m_scale.x, bc2d.Size.y * transform.m_scale.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape                = &boxShape;
            fixtureDef.density              = bc2d.Density;
            fixtureDef.friction             = bc2d.Friction;
            fixtureDef.restitution          = bc2d.Restitution;
            fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
            body->CreateFixture(&fixtureDef);
        }
    }
}

void Scene::OnRuntimeStop()
{
    delete m_physicsWorld;
    m_physicsWorld = nullptr;
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
    // Physics
    {
        const int32_t velocityIterations = 6;
        const int32_t positionIterations = 2;
        m_physicsWorld->Step(ts, velocityIterations, positionIterations);
        // Retrieve transform from Box2D
        auto view = m_registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity    = {e, this};
            auto&  transform = entity.GetComponent<TransformComponent>();
            auto&  rb2d      = entity.GetComponent<Rigidbody2DComponent>();

            b2Body*     body          = (b2Body*)rb2d.RuntimeBody;
            const auto& position      = body->GetPosition();
            transform.m_translation.x = position.x;
            transform.m_translation.y = position.y;
            transform.m_rotation.z    = body->GetAngle();
        }
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
        // Draw sprite
        {
            auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
            }
        }
        // Draw circle
        {
            auto view = m_registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view)
            {
                auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                                       static_cast<int>(entity));
            }
        }
        Renderer2D::EndScene();
    }
}

void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
{
    Renderer2D::BeginScene(camera);
    // Draw sprite
    auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
    for (auto entity : group)
    {
        auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
        Renderer2D::DrawSprite(transform.GetTransform(), sprite, static_cast<int>(entity));
    }
    // Draw circle
    auto view = m_registry.view<TransformComponent, CircleRendererComponent>();
    for (auto entity : view)
    {
        auto [transform, circle] = group.get<TransformComponent, CircleRendererComponent>(entity);
        Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                               static_cast<int>(entity));
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

void Scene::DuplicateEntity(Entity entity)
{
    std::string name      = entity.GetName();
    Entity      newEntity = CreateEntity(name);
    CopyComponentIfExists<TransformComponent>(newEntity, entity);
    CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
    CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
    CopyComponentIfExists<CameraComponent>(newEntity, entity);
    CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
    CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
    CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
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

// 模板特化
template <>
void Scene::onComponentAdded<IDComponent>(Entity entity, IDComponent& component)
{
}

template <>
void Scene::onComponentAdded<TagComponent>(Entity entity, TagComponent& component)
{
}

template <>
void Scene::onComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
{
}

template <>
void Scene::onComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
{
}

template <>
void Scene::onComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
{
}

template <>
void Scene::onComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
{
    if (m_viewportWidth > 0 && m_viewportHeight > 0)
    {
        component.m_camera.SetViewportSize(m_viewportWidth, m_viewportHeight);
    }
}

template <>
void Scene::onComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
{
}

template <>
void Scene::onComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
{
}

template <>
void Scene::onComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
{
}