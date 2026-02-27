//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "pch.h"
#include "x/core.h"

// interface
class Texture {
public:
    virtual ~Texture() = default;

    virtual uint32_t GetWidth() const =0;

    virtual uint32_t GetHeight() const =0;

    // texture uint
    virtual void Bind(uint32_t slot = 0) const =0;

protected:
    Texture() = default;
};

class Texture2D : public Texture {
public:
    static X::Ref<Texture2D> Create(const std::string &path);
};
