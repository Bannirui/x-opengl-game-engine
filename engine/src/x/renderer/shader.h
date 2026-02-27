//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"

class Shader
{
public:
    Shader(const std::string &vertexSrc, const std::string fragmentSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

private:
    uint32_t m_rendererId{0};
};
