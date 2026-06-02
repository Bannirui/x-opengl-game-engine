//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

class Shader;
class Texture2D;

class Material {
public:
    static X::Ref<Material> Create(const X::Ref<Shader>& shader);

    void Bind() const;
    void Unbind() const;

    void SetTexture(const std::string& name, const X::Ref<Texture2D>& texture);
    void SetFloat(const std::string& name, float value);
    void SetFloat3(const std::string& name, const glm::vec3& value);
    void SetFloat4(const std::string& name, const glm::vec4& value);

    const X::Ref<Shader>& GetShader() const { return m_shader; }
    const std::unordered_map<std::string, X::Ref<Texture2D>>& GetTextures() const { return m_textures; }

    bool operator==(const Material& other) const;

    Material() = default;

public:

    X::Ref<Shader> m_shader;
    std::unordered_map<std::string, X::Ref<Texture2D>> m_textures;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, glm::vec3> m_vec3s;
    std::unordered_map<std::string, glm::vec4> m_vec4s;
};
