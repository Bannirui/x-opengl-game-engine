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
    void SetInt(const std::string& name, int value) override;
    void SetIntArray(const std::string& name, int* values, int count) override;
    void SetFloat(const std::string& name, float value) override;
    void SetFloat2(const std::string& name, const glm::vec2& value) override;
    void SetFloat3(const std::string& name, const glm::vec3& value) override;
    void SetFloat4(const std::string& name, const glm::vec4& value) override;
    void SetMat4(const std::string& name, const glm::mat4& value) override;
    // --- 开放的接口 传递uniform变量---

private:
    void uploadUniformInt(const std::string& name, int value);
    void uploadUniformIntArray(const std::string& name, int* values, uint32_t count);
    void uploadUniformFloat(const std::string& name, float value);
    void uploadUniformFloat2(const std::string& name, const glm::vec2& value);
    void uploadUniformFloat3(const std::string& name, const glm::vec3& value);
    void uploadUniformFloat4(const std::string& name, const glm::vec4& value);
    void uploadUniformMat3(const std::string& name, const glm::mat3& matrix);
    void uploadUniformMat4(const std::string& name, const glm::mat4& matrix);

private:
    // 从文件读源码
    std::string readFile(const std::string& filepath);
    // 源码区分vertex和frag
    std::unordered_map<GLenum, std::string> preProcess(const std::string& glslSrc);
    void compileOrGetBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
    void creatProgram();
    void reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

private:
    uint32_t m_rendererId{0};
    /**
     * 创建Shader程序时候传捡来的路径可能是x.glsl
     * 拿到这个路径并不是直接就用它的源码了
     *   - 1 先拼接上运行时的OpenGL版本成为类似x.330.glsl路径
     *   - 2 找不到带版本的源码再用找传进来的路径
     * 达到了根据运行时OpenGL版本动态适配GLSL语法的效果
     */
    std::string m_filePath;
    std::string m_name;
    // GLSL->Shaderc->Spir-V字节码 把GLSL编译成了spirv字节码 高版本支持spirv就用这种
    std::unordered_map<GLenum, std::vector<uint32_t>> m_spirvBinaries;
    // GLSL源码 低版本不支持spirv就用源码
    std::unordered_map<GLenum, std::string> m_glslSources;
};
