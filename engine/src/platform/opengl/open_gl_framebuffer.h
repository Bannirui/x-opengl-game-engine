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
    uint32_t                        GetColorAttachmentRendererID() const override { return m_colorAttachment; }
    const FramebufferSpecification &GetSpecification() const override { return m_specification; }

private:
    uint32_t                 m_rendererID;
    uint32_t                 m_colorAttachment, m_depthAttachment;
    FramebufferSpecification m_specification;
};
