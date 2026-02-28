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

    /**
     * @param filepath glsl源码路径 vertex跟frag在同一个文件 用type区分
     */
    static Shader *Create(const std::string &filepath);
    /**
     * @param vertexSrc vertex glsl源码
     * @param fragmentSrc frag glsl源码
     */
    static Shader *Create(const std::string &vertexSrc, const std::string &fragmentSrc);

protected:
    Shader() = default;
};
