//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/core/base.h"

enum class FramebufferTextureFormat : uint8_t
{
    kNone = 0,
    kRGBA8,  // Color
    kRED_INTEGER,
    kDEPTH24STENCIL8,           // depth/stencil
    kDepth = kDEPTH24STENCIL8,  // default
};

struct FramebufferTextureSpecification
{
    FramebufferTextureSpecification() = default;

    FramebufferTextureSpecification(FramebufferTextureFormat format) : m_textureFormat(format) {}

    FramebufferTextureFormat m_textureFormat = FramebufferTextureFormat::kNone;
};

struct FramebufferAttachmentSpecification
{
    FramebufferAttachmentSpecification() = default;

    FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
        : m_attachments(attachments)
    {
    }

    std::vector<FramebufferTextureSpecification> m_attachments;
};

struct FramebufferSpecification
{
    uint32_t                           m_width{0}, m_height{0};
    FramebufferAttachmentSpecification m_attachments;
    uint32_t                           m_samples{1};
    bool                               m_swapChainTarget{false};
};

class FrameBuffer
{
protected:
    FrameBuffer() = default;

public:
    virtual ~FrameBuffer() = default;

    virtual void Bind()   = 0;
    virtual void Unbind() = 0;

    virtual void     Resize(uint32_t width, uint32_t height)                = 0;
    virtual int      ReadPixel(uint32_t attachmentIndex, int x, int y)      = 0;
    virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

    virtual const FramebufferSpecification& GetSpecification() const = 0;

    static X::Ref<FrameBuffer> Create(const FramebufferSpecification& spec);
};
