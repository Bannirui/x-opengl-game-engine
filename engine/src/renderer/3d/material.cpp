//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/material.h"

#include "x/renderer/shader.h"
#include "x/renderer/texture.h"

X::Ref<Material> Material::Create(const X::Ref<Shader>& shader) {
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(shader, "Material requires a valid shader");
    X::Ref<Material> material = X::CreateRef<Material>();
    material->m_shader = shader;
    return material;
}

void Material::Bind() const {
    // 着色器Bind 准备给着色器程序传uniform变量
    m_shader->Bind();
    // 传纹理采样器
    int slot = 0;
    for (const auto& [name, texture] : m_textures) {
        texture->Bind(slot);
        m_shader->SetInt(name, slot);
        ++slot;
    }
    // 传float
    for (const auto& [name, value] : m_floats) {
        m_shader->SetFloat(name, value);
    }
    // 传向量
    for (const auto& [name, value] : m_vec3s) {
        m_shader->SetFloat3(name, value);
    }
    // 传矩阵
    for (const auto& [name, value] : m_vec4s) {
        m_shader->SetFloat4(name, value);
    }
}

void Material::Unbind() const {
    m_shader->Unbind();
}

void Material::SetTexture(const std::string& name, const X::Ref<Texture2D>& texture) {
    m_textures[name] = texture;
}

void Material::SetFloat(const std::string& name, float value) {
    m_floats[name] = value;
}

void Material::SetFloat3(const std::string& name, const glm::vec3& value) {
    m_vec3s[name] = value;
}

void Material::SetFloat4(const std::string& name, const glm::vec4& value) {
    m_vec4s[name] = value;
}

bool Material::operator==(const Material& other) const {
    if (m_lightGroupId != other.m_lightGroupId) return false;
    if (m_shader.get() != other.m_shader.get()) return false;
    if (m_textures.size() != other.m_textures.size()) return false;
    for (const auto& [name, texture] : m_textures) {
        auto it = other.m_textures.find(name);
        if (it == other.m_textures.end()) return false;
        if (texture.get() != it->second.get()) return false;
    }
    return true;
}
