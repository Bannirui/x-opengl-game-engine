//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "pch.h"

// interface
class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const =0;

    virtual void Unbind() const =0;

    static Shader *Create(const std::string &vertexSrc, const std::string &fragmentSrc);

protected:
    Shader() = default;
};
