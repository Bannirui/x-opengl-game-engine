//
// Created by dingrui on 3/4/26.
//

#pragma once

class Scene;

class SceneSerializer
{
public:
    SceneSerializer(const X::Ref<Scene>& scene);

    void Serialize(const std::string& filepath);
    void SerializeRuntime(const std::string& filepath);

    bool Deserialize(const std::string& filepath);
    bool DeserializeRuntime(const std::string& filepath);

private:
    X::Ref<Scene> m_scene;
};
