//
// Created by dingrui on 3/3/26.
//

#include "platform/opengl/open_gl_framebuffer.h"

#include <glad/glad.h>

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec) : m_specification(spec)
{
    Invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    if (m_rendererID != 0)
    {
        glDeleteFramebuffers(1, &m_rendererID);
    }
}

void OpenGLFramebuffer::Invalidate()
{
    glGenFramebuffers(1, &m_rendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

    glGenTextures(1, &m_colorAttachment);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_specification.width, m_specification.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorAttachment, 0);

    glGenTextures(1, &m_depthAttachment);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_specification.width, m_specification.height, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
}

void OpenGLFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
