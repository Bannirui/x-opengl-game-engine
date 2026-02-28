//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "pch.h"

#include <glm/glm.hpp>

#include "x/renderer/shader.h"

class OpenGLShader : public Shader {
public:
    /**
     * @param filepath glsl源码路径 vertex跟frag在同一个文件 用type区分
     */
    OpenGLShader(const std::string &filepath);
    /**
     * @name 项目里面会有很多glsl shader程序 给每个一个文件名
     * @param vertexSrc vertex glsl源码
     * @param fragmentSrc frag glsl源码
     */
    OpenGLShader(const std::string& name, const std::string &vertexSrc, const std::string fragmentSrc);
    ~OpenGLShader() override;

    const std::string& get_name() const override { return m_name; }

    void Bind() const override;

    void Unbind() const override;

    void UploadUniformInt(const std::string &name, int value);

    void UploadUniformFloat(const std::string &name, float value);

    void UploadUniformFloat2(const std::string &name, const glm::vec2 &value);

    void UploadUniformFloat3(const std::string &name, const glm::vec3 &value);

    void UploadUniformFloat4(const std::string &name, const glm::vec4 &value);

    void UploadUniformMat3(const std::string &name, const glm::mat3 &matrix);

    void UploadUniformMat4(const std::string &name, const glm::mat4 &matrix);

private:
    // 从文件读源码
    std::string readFile(const std::string& filepath);
    // 源码区分vertex和frag
    std::unordered_map<int, std::string> preProcess(const std::string& glslSrc);
    // vertex跟frag编译
    void compile(const std::unordered_map<int, std::string>& shaderSources);

private:
    uint32_t m_rendererId{0};
    std::string m_name;
};
