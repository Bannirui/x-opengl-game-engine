//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "pch.h"

#include "x/renderer/frame_buffer.h"

class OpenGLFramebuffer : public FrameBuffer
{
public:
    OpenGLFramebuffer(const FramebufferSpecification& spec);
    ~OpenGLFramebuffer() override;

    void Invalidate();

    void     Bind() override;
    void     Unbind() override;
    void     Resize(uint32_t width, uint32_t height) override;
    int      ReadPixel(uint32_t attachmentIndex, int x, int y) override;
    uint32_t GetColorAttachmentRendererID(uint32_t index) const override;

    const FramebufferSpecification& GetSpecification() const override { return m_specification; }

private:
    uint32_t                 m_rendererID{0};
    FramebufferSpecification m_specification;

    std::vector<FramebufferTextureSpecification> m_colorAttachmentSpecifications;
    std::vector<uint32_t>                        m_colorAttachments;

    FramebufferTextureSpecification m_depthAttachmentSpecification = FramebufferTextureFormat::kNone;
    uint32_t                        m_depthAttachment{0};
};
