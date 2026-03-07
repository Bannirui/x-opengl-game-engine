//
// Created by dingrui on 3/7/26.
//

#pragma once

#include "x/core/base.h"

class UniformBuffer
{
public:
    virtual ~UniformBuffer()                                                   = default;
    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

    static X::Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);
};
