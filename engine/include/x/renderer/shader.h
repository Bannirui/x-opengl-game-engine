//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <unordered_map>

/**
 * 为了让OpenGL能够用shader的代码 要在运行时动态地编译glsl源码
 * OpenGL创建个shader object然后分配个id引用它
 */
class Shader {
protected:
    Shader() = default;

public:
    virtual ~Shader() = default;

    virtual const std::string& get_name() const = 0;
    virtual uint32_t GetRendererID() const = 0;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    // --- uniform variable ---
    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetIntArray(const std::string& name, int* values, int count) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
    virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
    virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
    virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
    virtual bool HasUniform(const std::string& name) const = 0;
    // --- uniform variable ---

    /**
     * shader源码里面
     * #type vertex 标识vertex程序
     * #type fragment 标识frag程序
     * @param filepath glsl源码路径 vertex跟frag在同一个文件 用type区分
     */
    static Ref<Shader> Create(const std::string& filepath);
    /**
     * @param name shader glsl程序文件名
     * @param vertexSrc vertex glsl源码
     * @param fragmentSrc frag glsl源码
     */
    static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
};

class ShaderLib {
public:
    void Add(const std::string& name, const Ref<Shader>& shader);
    void Add(const Ref<Shader>& shader);
    Ref<Shader> Load(const std::string& filepath);
    Ref<Shader> Load(const std::string& name, const std::string& filepath);
    Ref<Shader> Get(const std::string& name);
    bool Exists(const std::string& name) const;

private:
    // shader缓存在这
    std::unordered_map<std::string, Ref<Shader>> m_shaders;
};
