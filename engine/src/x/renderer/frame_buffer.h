//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/core/base.h"

struct FramebufferSpecification
{
    uint32_t width, height;
    uint32_t samples{1};
    bool     swapChainTarget{false};
};

class FrameBuffer
{
protected:
    FrameBuffer() = default;

public:
    virtual ~FrameBuffer() = default;

    virtual void Bind()   = 0;
    virtual void Unbind() = 0;

    virtual uint32_t GetColorAttachmentRendererID() const = 0;

    virtual const FramebufferSpecification &GetSpecification() const = 0;

    static X::Ref<FrameBuffer> Create(const FramebufferSpecification &spec);
};
