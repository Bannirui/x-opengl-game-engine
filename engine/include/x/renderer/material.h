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

/**
 * 参数字典+绑定器
 * 用哪个shader+上传什么参数
 * 运行时通过Bind一次性激活shader并上传所有的纹理/数值到GPU
 */
class Material {
public:
    /**
     * 静态工厂
     * @param shader 着色器程序
     */
    static X::Ref<Material> Create(const X::Ref<Shader>& shader);

    /**
     * 给shader程序传uniform变量
     *   - 纹理采样器
     *   - float
     *   - 向量
     *   - 矩阵
     */
    void Bind() const;
    void Unbind() const;
    /**
     * 给shader程序传纹理采样器
     * @param name uniform变量名
     * @param texture 纹理对象
     */
    void SetTexture(const std::string& name, const X::Ref<Texture2D>& texture);
    /**
     * 给shader程序传float
     * @param name uniform变量名
     * @param value float值
     */
    void SetFloat(const std::string& name, float value);
    /**
     * 给shader程序传向量
     * @param name uniform变量名
     * @param value 向量
     */
    void SetFloat3(const std::string& name, const glm::vec3& value);
    /**
     * 给shader程序传矩阵
     * @param name uniform变量名
     * @param value 矩阵
     */
    void SetFloat4(const std::string& name, const glm::vec4& value);

    const X::Ref<Shader>& GetShader() const {
        return m_shader;
    }

    const std::unordered_map<std::string, X::Ref<Texture2D>>& GetTextures() const {
        return m_textures;
    }

    bool operator==(const Material& other) const;

    Material() = default;

public:
    // 对应的着色器程序
    X::Ref<Shader> m_shader;
    // 纹理映射表 uniform名->纹理 调用方自己决定顺序 对应的就是shader里面的binding slot顺序
    std::unordered_map<std::string, X::Ref<Texture2D>> m_textures;
    std::unordered_map<std::string, float> m_floats;
    std::unordered_map<std::string, glm::vec3> m_vec3s;
    std::unordered_map<std::string, glm::vec4> m_vec4s;
};
