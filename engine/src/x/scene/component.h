//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "pch.h"
#include "x/core/uuid.h"
#include "x/scene/scene_camera.h"

class ScriptableEntity;
class Texture2D;

struct IDComponent
{
    UUID id;

    IDComponent()                   = default;
    IDComponent(const IDComponent&) = default;
};

struct TagComponent
{
    std::string m_tag;

    TagComponent()                    = default;
    TagComponent(const TagComponent&) = default;

    TagComponent(const std::string& tag) : m_tag(tag) {}
};

struct TransformComponent
{
    glm::vec3 Translation{0.0f};
    glm::vec3 Rotation{0.0f};
    glm::vec3 Scale{1.0f};

    TransformComponent()                          = default;
    TransformComponent(const TransformComponent&) = default;

    TransformComponent(const glm::vec3& translation) : Translation(translation) {}

    glm::mat4 GetTransform() const
    {
        glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
        return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct SpriteRendererComponent
{
    glm::vec4         Color{1.0f};
    X::Ref<Texture2D> Texture;
    float             TilingFactor{1.0f};

    SpriteRendererComponent()                               = default;
    SpriteRendererComponent(const SpriteRendererComponent&) = default;

    SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
};

struct CircleRendererComponent
{
    glm::vec4 Color{1.0f};
    float     Thickness{1.0f};
    float     Fade{0.005f};

    CircleRendererComponent()                               = default;
    CircleRendererComponent(const CircleRendererComponent&) = default;
};

struct CameraComponent
{
    SceneCamera Camera;
    bool        Primary = true;
    bool        FixedAspectRatio{false};

    CameraComponent()                       = default;
    CameraComponent(const CameraComponent&) = default;
};

struct NativeScriptComponent
{
    ScriptableEntity* Instance = nullptr;
    ScriptableEntity* (*InstantiateScript)();
    void (*DestroyScript)(NativeScriptComponent*);

    template <typename T>
    void Bind()
    {
        InstantiateScript = []()
        {
            return static_cast<ScriptableEntity*>(new T());
        };
        DestroyScript = [](NativeScriptComponent* nsc)
        {
            delete nsc->Instance;
            nsc->Instance = nullptr;
        };
    }
};

// Physics
struct Rigidbody2DComponent
{
    enum class BodyType
    {
        Static = 0,
        Dynamic,
        Kinematic
    };
    BodyType Type = BodyType::Static;
    bool     FixedRotation{false};
    void*    RuntimeBody{nullptr};

    Rigidbody2DComponent()                            = default;
    Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
};

struct BoxCollider2DComponent
{
    glm::vec2 Offset = {0.0f, 0.0f};
    glm::vec2 Size   = {0.5f, 0.5f};

    float Density{1.0f};
    float Friction{0.5f};
    float Restitution{0.0f};
    float RestitutionThreshold{0.5f};

    void* RuntimeFixture{nullptr};

    BoxCollider2DComponent()                              = default;
    BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
};

struct CircleCollider2DComponent
{
    glm::vec2 Offset{0.0f, 0.0f};
    float     Radius{0.5f};

    float Density{1.0f};
    float Friction{0.5f};
    float Restitution{0.0f};
    float RestitutionThreshold{0.5f};

    void* RuntimeFixture{nullptr};

    CircleCollider2DComponent()                                 = default;
    CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
};

template <typename... Component>
struct ComponentGroup
{
};

using AllComponents =
    ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent, CameraComponent,
                   NativeScriptComponent, Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;