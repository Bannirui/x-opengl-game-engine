//
// Created by dingrui on 3/4/26.
//

#include "x/scene/scene_serializer.h"

#include <x/scene/component.h>
#include <x/scene/entity.h>

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML
{
    template <>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
            {
                return false;
            }
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
            {
                return false;
            }
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}  // namespace YAML

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

static void serializeEntity(YAML::Emitter& out, Entity entity)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Entity" << YAML::Value << "123456789";  // todo
    if (entity.HasComponent<TagComponent>())
    {
        out << YAML::Key << "TagComponent";
        out << YAML::BeginMap;
        auto& tag = entity.GetComponent<TagComponent>().m_tag;
        out << YAML::Key << "Tag" << YAML::Value << tag;
        out << YAML::EndMap;
    }
    if (entity.HasComponent<TransformComponent>())
    {
        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap;
        auto& tc = entity.GetComponent<TransformComponent>();
        out << YAML::Key << "Translation" << YAML::Value << tc.m_translation;
        out << YAML::Key << "Rotation" << YAML::Value << tc.m_rotation;
        out << YAML::Key << "Scale" << YAML::Value << tc.m_scale;
        out << YAML::EndMap;
    }
    if (entity.HasComponent<CameraComponent>())
    {
        out << YAML::Key << "CameraComponent";
        out << YAML::BeginMap;
        auto& cc     = entity.GetComponent<CameraComponent>();
        auto& camera = cc.m_camera;
        out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.get_projectionType();
        out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.get_perspectiveFOV();
        out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.get_perspectiveNearClip();
        out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.get_perspectiveFarClip();
        out << YAML::Key << "OrthographicSize" << YAML::Value << camera.get_orthographicSize();
        out << YAML::Key << "OrthographicNear" << YAML::Value << camera.get_orthographicNearClip();
        out << YAML::Key << "OrthographicFar" << YAML::Value << camera.get_orthographicFarClip();
        out << YAML::EndMap;
        out << YAML::Key << "Primary" << YAML::Value << cc.m_primary;
        out << YAML::Key << "FixedAspectiveRatio" << YAML::Value << cc.m_fixedAspectRatio;
        out << YAML::EndMap;
    }
    if (entity.HasComponent<SpriteRendererComponent>())
    {
        out << YAML::Key << "SpriteRendererComponent";
        out << YAML::BeginMap;
        auto& sc = entity.GetComponent<SpriteRendererComponent>();
        out << YAML::Key << "Color" << YAML::Value << sc.m_color;
        out << YAML::EndMap;
    }
    out << YAML::EndMap;
}

SceneSerializer::SceneSerializer(const X::Ref<Scene>& scene) : m_scene(scene) {}

void SceneSerializer::Serialize(const std::string& filepath)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Untitled";
    out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
    for (auto entityID : m_scene->m_registry.view<entt::entity>())
    {
        Entity entity{entityID, m_scene.get()};
        if (!entity)
        {
            return;
        }
        serializeEntity(out, entity);
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;
    std::ofstream fout(filepath);
    fout << out.c_str();
}

void SceneSerializer::SerializeRuntime(const std::string& filepath)
{
    X_CORE_ASSERT(false);
}

bool SceneSerializer::Deserialize(const std::string& filepath)
{
    std::ifstream     in(filepath);
    std::stringstream buffer;
    buffer << in.rdbuf();

    YAML::Node data = YAML::Load(buffer.str());
    if (!data["Scene"])
    {
        return false;
    }
    std::string sceneName = data["Scene"].as<std::string>();
    X_CORE_TRACE("Deserializing scene '{0}'", sceneName);
    auto entities = data["Entities"];
    if (!entities)
    {
        return true;
    }
    for (auto entity : entities)
    {
        uint64_t    uuid = entity["Entity"].as<uint64_t>();
        std::string name;
        auto        tagComponent = entity["TagComponent"];
        if (tagComponent)
        {
            name = tagComponent["Tag"].as<std::string>();
            X_CORE_TRACE("Deserializing entity with ID = {0}, name = {1}", uuid, name);
            Entity deserializeEntity  = m_scene->CreateEntity(name);
            auto   transformComponent = entity["TransformComponent"];
            if (transformComponent)
            {
                auto& tc         = deserializeEntity.GetComponent<TransformComponent>();
                tc.m_translation = transformComponent["Translation"].as<glm::vec3>();
                tc.m_rotation    = transformComponent["Rotation"].as<glm::vec3>();
                tc.m_scale       = transformComponent["Scale"].as<glm::vec3>();
            }
            auto cameraComponent = entity["CameraComponent"];
            if (cameraComponent)
            {
                auto&       cc          = deserializeEntity.AddComponent<CameraComponent>();
                const auto& cameraProps = cameraComponent["Camera"];
                cc.m_camera.set_projectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
                cc.m_camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
                cc.m_camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                cc.m_camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
                cc.m_camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                cc.m_camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                cc.m_camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());
                cc.m_primary          = cameraComponent["Primary"].as<bool>();
                cc.m_fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
            }
            auto spriteRendererComponent = entity["SpriteRendererComponent"];
            if (spriteRendererComponent)
            {
                auto& src   = deserializeEntity.AddComponent<SpriteRendererComponent>();
                src.m_color = spriteRendererComponent["Color"].as<glm::vec4>();
            }
        }
    }
    return true;
}

bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
{
    X_CORE_ASSERT(false);
    return false;
}
