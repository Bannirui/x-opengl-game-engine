//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"

#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string &vertexSrc, const std::string fragmentSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void UploadUniformMat4(const std::string &name, const glm::mat4& matrix);

private:
    uint32_t m_rendererId{0};
};
