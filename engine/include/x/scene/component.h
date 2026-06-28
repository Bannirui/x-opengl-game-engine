//
// Created by rui ding on 2026/3/3.
//

#pragma once

#include "x/animation/animator.h"
#include "x/core/base.h"
#include "x/core/uuid.h"
#include "x/renderer/camera/scene_camera.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Animator;
class Material;
class Mesh;
class ScriptableEntity;
class Texture2D;

struct IDComponent {
    UUID ID;

    IDComponent() = default;
    IDComponent(const IDComponent&) = default;
    IDComponent& operator=(const IDComponent&) = default;
};

struct TagComponent {
    std::string m_tag;

    TagComponent() = default;
    TagComponent(const TagComponent&) = default;
    TagComponent& operator=(const TagComponent&) = default;

    TagComponent(const std::string& tag) : m_tag(tag) {}
};

struct TransformComponent {
    glm::vec3 Translation{0.0f};
    glm::vec3 Rotation{0.0f};
    glm::vec3 Scale{1.0f};

    TransformComponent() = default;
    TransformComponent(const TransformComponent&) = default;

    TransformComponent(const glm::vec3& translation) : Translation(translation) {}

    glm::mat4 GetTransform() const {
        glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
        return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
    }
};

struct SpriteRendererComponent {
    glm::vec4 Color{1.0f};
    Ref<Texture2D> Texture;
    float TilingFactor{1.0f};

    SpriteRendererComponent() = default;
    SpriteRendererComponent(const SpriteRendererComponent&) = default;

    SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
};

struct CircleRendererComponent {
    glm::vec4 Color{1.0f};
    float Thickness{1.0f};
    float Fade{0.005f};

    CircleRendererComponent() = default;
    CircleRendererComponent(const CircleRendererComponent&) = default;
};

struct MeshComponent {
    Ref<Mesh> MeshHandle;

    MeshComponent() = default;
    MeshComponent(const MeshComponent&) = default;
};

struct MaterialComponent {
    Ref<Material> MaterialHandle;

    MaterialComponent() = default;
    MaterialComponent(const MaterialComponent&) = default;
};

struct CameraComponent {
    SceneCamera Camera;
    bool Primary = true;
    bool FixedAspectRatio{false};

    CameraComponent() = default;
    CameraComponent(const CameraComponent&) = default;
};

struct ScriptComponent {
    std::string ClassName;

    ScriptComponent() = default;
    ScriptComponent(const ScriptComponent&) = default;
};

// 前向声明
class ScriptableEntity;

struct NativeScriptComponent {
    ScriptableEntity* Instance = nullptr;
    ScriptableEntity* (*InstantiateScript)();
    void (*DestroyScript)(NativeScriptComponent*);

    template <typename T>
    void Bind() {
        InstantiateScript = []() {
            return static_cast<ScriptableEntity*>(new T());
        };
        DestroyScript = [](NativeScriptComponent* nsc) {
#ifdef X_PLATFORM_MAC
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdelete-incomplete"
#endif
            // mac上 前向声明信息不完整delete 编译不过
            delete nsc->Instance;
#ifdef X_PLATFORM_MAC
    #pragma clang diagnostic pop
#endif
            nsc->Instance = nullptr;
        };
    }
};

// Physics
struct Rigidbody2DComponent {
    enum class BodyType { Static = 0, Dynamic, Kinematic };
    BodyType Type = BodyType::Static;
    bool FixedRotation{false};
    void* RuntimeBody{nullptr};

    Rigidbody2DComponent() = default;
    Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
};

struct BoxCollider2DComponent {
    glm::vec2 Offset = {0.0f, 0.0f};
    glm::vec2 Size = {0.5f, 0.5f};

    float Density{1.0f};
    float Friction{0.5f};
    float Restitution{0.0f};
    float RestitutionThreshold{0.5f};

    void* RuntimeFixture{nullptr};

    BoxCollider2DComponent() = default;
    BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
};

struct CircleCollider2DComponent {
    glm::vec2 Offset{0.0f, 0.0f};
    float Radius{0.5f};

    float Density{1.0f};
    float Friction{0.5f};
    float Restitution{0.0f};
    float RestitutionThreshold{0.5f};

    void* RuntimeFixture{nullptr};

    CircleCollider2DComponent() = default;
    CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
};

struct AnimatorComponent {
    Scope<Animator> AnimatorInstance;

    AnimatorComponent() = default;
    AnimatorComponent(const AnimatorComponent&) = delete;
    AnimatorComponent& operator=(const AnimatorComponent&) = delete;
    AnimatorComponent(AnimatorComponent&&) = default;
    AnimatorComponent& operator=(AnimatorComponent&&) = default;
};

enum class LightType : uint8_t { Directional = 0, Point = 1 };

struct LightComponent {
    LightType Type = LightType::Directional;
    glm::vec3 Color{1.0f};
    float Intensity = 1.0f;
    float Range = 10.0f;
    uint32_t LightGroupId = 0;

    LightComponent() = default;
    LightComponent(const LightComponent&) = default;
};

template <typename... Component>
struct ComponentGroup {};

using AllComponents =
    ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent, MeshComponent,
                   MaterialComponent, LightComponent, CameraComponent, ScriptComponent, NativeScriptComponent,
                   Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;