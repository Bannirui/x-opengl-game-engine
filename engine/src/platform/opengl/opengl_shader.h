//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/renderer/shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

class OpenGLShader : public Shader {
public:
    /**
     * shader源码里面
     * #type vertex 标识vertex程序
     * #type fragment 标识frag程序
     * 这个源码路径是{name}.glsl 拿到这个路径并不是直接就用它的源码了
     *   - 1 先拼接上运行时的OpenGL版本成为类似{name}.{version}.glsl路径
     *   - 2 要是找不到{name}.{version}.glsl 再用{name}.glsl
     * 达到了根据运行时OpenGL版本动态shader程序的效果
     * @param filepath glsl源码路径 vertex跟frag在同一个文件 用type区分
     */
    OpenGLShader(const std::string& filepath);

    /**
     * @name 项目里面会有很多glsl shader程序 给每个一个文件名
     * @param vertexSrc vertex glsl源码
     * @param fragmentSrc frag glsl源码
     */
    OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string fragmentSrc);

    ~OpenGLShader() override;

    const std::string& get_name() const override {
        return m_name;
    }

    void Bind() const override;
    void Unbind() const override;

    // --- 开放的接口 传递uniform变量---
    /**
     * @param name shader里面uniform变量名
     * @param value 要传的变量的值
     */
    void SetInt(const std::string& name, int value) override;
    void SetIntArray(const std::string& name, int* values, int count) override;
    void SetFloat(const std::string& name, float value) override;
    void SetFloat2(const std::string& name, const glm::vec2& value) override;
    void SetFloat3(const std::string& name, const glm::vec3& value) override;
    void SetFloat4(const std::string& name, const glm::vec4& value) override;
    void SetMat4(const std::string& name, const glm::mat4& value) override;
    // --- 开放的接口 传递uniform变量---

private:
    /**
     * @param name shader里面uniform变量名
     * @param value 要传的变量值
     */
    void uploadUniformInt(const std::string& name, int value) const;
    void uploadUniformIntArray(const std::string& name, int* values, uint32_t count) const;
    void uploadUniformFloat(const std::string& name, float value) const;
    void uploadUniformFloat2(const std::string& name, const glm::vec2& value) const;
    void uploadUniformFloat3(const std::string& name, const glm::vec3& value) const;
    void uploadUniformFloat4(const std::string& name, const glm::vec4& value) const;
    void uploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
    void uploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;

private:
    /**
     * 要是系统用的OpenGL版本高 就支持spirv 那么就用shaderc编译成spirv字节码
     * @param shaderSources vertex和frag源码 key是vertex和frag类型枚举 value是对应的源码
     */
    void compileBinariesIfSupportSpirv(const std::unordered_map<GLenum, std::string>& shaderSources);
    void creatProgram();
    void reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

private:
    // OpenGL创建的program object OpenGL会分配唯一的id引用它 以后渲染就是用这个program object
    // 只要先激活它就可以进行渲染了
    uint32_t m_rendererId{0};
    /**
     * 创建Shader程序时候传捡来的路径可能是{name}.glsl
     * 拿到这个路径并不是直接就用它的源码了
     *   - 1 先拼接上运行时的OpenGL版本成为{name}.{version}.glsl路径
     *   - 2 能找到{name}.{version}.glsl就用这个 找不到才用{name}.glsl
     * 达到了根据运行时OpenGL版本动态适配GLSL语法的效果
     */
    std::string m_filePath;
    // shader程序的名字 从shader程序路径里面截出来的
    std::string m_name;
    // GLSL->Shaderc->Spir-V字节码 把GLSL编译成了spirv字节码 高版本支持spirv就用这种
    std::unordered_map<GLenum, std::vector<uint32_t>> m_spirvBinaries;
    // GLSL源码 低版本不支持spirv就用源码
    std::unordered_map<GLenum, std::string> m_glslSources;
};
