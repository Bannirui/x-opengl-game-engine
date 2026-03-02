//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"
#include "x/core/core.h"

#include <glm/glm.hpp>

// interface
class Shader
{
public:
    virtual ~Shader() = default;

    virtual const std::string &get_name() const = 0;

    virtual void Bind() const = 0;

    virtual void Unbind() const = 0;

    // --- uniform variable ---
    virtual void SetInt(const std::string &name, int value)                 = 0;
    virtual void SetFloat(const std::string &name, float value)             = 0;
    virtual void SetFloat3(const std::string &name, const glm::vec3 &value) = 0;
    virtual void SetFloat4(const std::string &name, const glm::vec4 &value) = 0;
    virtual void SetMat4(const std::string &name, const glm::mat4 &value)   = 0;
    // --- uniform variable ---

    /**
     * @param filepath glsl源码路径 vertex跟frag在同一个文件 用type区分
     */
    static X::Ref<Shader> Create(const std::string &filepath);
    /**
     * @param name shader glsl程序文件名
     * @param vertexSrc vertex glsl源码
     * @param fragmentSrc frag glsl源码
     */
    static X::Ref<Shader> Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

protected:
    Shader() = default;
};

class ShaderLib
{
public:
    void           Add(const std::string &name, const X::Ref<Shader> &shader);
    void           Add(const X::Ref<Shader> &shader);
    X::Ref<Shader> Load(const std::string &filepath);
    X::Ref<Shader> Load(const std::string &name, const std::string &filepath);
    X::Ref<Shader> Get(const std::string &name);
    bool           Exists(const std::string &name) const;

private:
    std::unordered_map<std::string, X::Ref<Shader>> m_shaders;
};
