//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "pch.h"
#include "x/core/core.h"

// interface
class Texture
{
public:
    virtual ~Texture() = default;

    virtual uint32_t GetWidth() const  = 0;
    virtual uint32_t GetHeight() const = 0;

    /**
     * @param data pointer of data
     * @param size how many bytes of data
     */
    virtual void SetData(void *data, uint32_t size) = 0;

    // texture uint
    virtual void Bind(uint32_t slot = 0) const = 0;

protected:
    Texture() = default;
};

class Texture2D : public Texture
{
public:
    static X::Ref<Texture2D> Create(const std::string &path);
    static X::Ref<Texture2D> Create(uint32_t width, uint32_t height);
};
