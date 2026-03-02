//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/renderer/frame_buffer.h"

class OpenGLFramebuffer : public FrameBuffer
{
public:
    OpenGLFramebuffer(const FramebufferSpecification &spec);
    ~OpenGLFramebuffer() override;

    void Invalidate();

    void                            Bind() override;
    void                            Unbind() override;
    void                            Resize(uint32_t width, uint32_t height) override;
    uint32_t                        GetColorAttachmentRendererID() const override { return m_colorAttachment; }
    const FramebufferSpecification &GetSpecification() const override { return m_specification; }

private:
    uint32_t                 m_rendererID{0};
    uint32_t                 m_colorAttachment{0}, m_depthAttachment{0};
    FramebufferSpecification m_specification;
};
