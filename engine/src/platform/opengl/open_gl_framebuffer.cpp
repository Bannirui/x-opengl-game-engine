//
// Created by dingrui on 3/3/26.
//

#include "platform/opengl/open_gl_framebuffer.h"

#include <glad/glad.h>

static const uint32_t s_MaxFramebufferSize = 8192;

namespace Util
{
    static GLenum TextureTarget(bool multisampled)
    {
        return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    }

    static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
    {
        glGenTextures(count, outID);
        for (uint32_t i = 0; i < count; i++)
        {
            glBindTexture(TextureTarget(multisampled), outID[i]);
        }
    }

    static void BindTexture(bool multisampled, uint32_t id)
    {
        glBindTexture(TextureTarget(multisampled), id);
    }

    static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width,
                                   uint32_t height, int index)
    {
        bool   multisampled = samples > 1;
        GLenum target       = TextureTarget(multisampled);
        glBindTexture(target, id);
        if (multisampled)
        {
            glTexImage2DMultisample(target, samples, internalFormat, width, height, GL_FALSE);
        }
        else
        {
            glTexImage2D(target, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, id, 0);
    }

    static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width,
                                   uint32_t height)
    {
        bool   multisampled = samples > 1;
        GLenum target       = TextureTarget(multisampled);
        glBindTexture(target, id);
        if (multisampled)
        {
            glTexImage2DMultisample(target, samples, format, width, height, GL_FALSE);
        }
        else
        {
            glTexImage2D(target, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, target, id, 0);
    }

    static bool IsDepthFormat(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::kDEPTH24STENCIL8:
                return true;
            default:
                break;
        }
        return false;
    }
}  // namespace Util

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_specification(spec)
{
    for (auto attach : m_specification.m_attachments.m_attachments)
    {
        if (!Util::IsDepthFormat(attach.m_textureFormat))
        {
            m_colorAttachmentSpecifications.emplace_back(attach);
        }
        else
        {
            m_depthAttachmentSpecification = attach;
        }
    }
    Invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer()
{
    if (m_rendererID)
    {
        glDeleteFramebuffers(1, &m_rendererID);
    }
    if (!m_colorAttachments.empty())
    {
        glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
    }
    if (m_depthAttachment)
    {
        glDeleteTextures(1, &m_depthAttachment);
    }
}

void OpenGLFramebuffer::Invalidate()
{
    // Cleanup old framebuffer
    if (m_rendererID)
    {
        glDeleteFramebuffers(1, &m_rendererID);
        if (!m_colorAttachments.empty())
        {
            glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
            m_colorAttachments.clear();
        }
        if (m_depthAttachment)
        {
            glDeleteTextures(1, &m_depthAttachment);
            m_depthAttachment = 0;
        }
    }
    // Create framebuffer
    glGenFramebuffers(1, &m_rendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

    bool multisampled = m_specification.m_samples > 1;
    // Attachments
    if (!m_colorAttachmentSpecifications.empty())
    {
        m_colorAttachments.resize(m_colorAttachmentSpecifications.size());
        Util::CreateTextures(multisampled, m_colorAttachments.data(), m_colorAttachments.size());
        for (size_t i = 0; i < m_colorAttachments.size(); i++)
        {
            Util::BindTexture(multisampled, m_colorAttachments[i]);
            switch (m_colorAttachmentSpecifications[i].m_textureFormat)
            {
                case FramebufferTextureFormat::kRGBA8:
                    Util::AttachColorTexture(m_colorAttachments[i], m_specification.m_samples, GL_RGBA8, GL_RGBA,
                                             m_specification.m_width, m_specification.m_height, i);
                    break;
                case FramebufferTextureFormat::kRED_INTEGER:
                    Util::AttachColorTexture(m_colorAttachments[i], m_specification.m_samples, GL_R32I, GL_RED_INTEGER,
                                             m_specification.m_width, m_specification.m_height, i);
                default:
                    X_CORE_ASSERT(false, "Unknown framebuffer format")
            }
        }
    }
    if (m_depthAttachmentSpecification.m_textureFormat != FramebufferTextureFormat::kNone)
    {
        Util::CreateTextures(multisampled, &m_depthAttachment, 1);
        Util::BindTexture(multisampled, m_depthAttachment);
        switch (m_depthAttachmentSpecification.m_textureFormat)
        {
            case FramebufferTextureFormat::kDEPTH24STENCIL8:
                Util::AttachDepthTexture(m_depthAttachment, m_specification.m_samples, GL_DEPTH24_STENCIL8,
                                         GL_DEPTH_STENCIL_ATTACHMENT, m_specification.m_width,
                                         m_specification.m_height);
                break;
            default:
                X_CORE_ASSERT(false, "Unknown depth format")
        }
    }
    if (m_colorAttachments.empty())
    {
        // Only depth-pass
        glDrawBuffer(GL_NONE);
    }
    else
    {
        X_CORE_ASSERT(m_colorAttachments.size() <= 4);
        GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(m_colorAttachments.size(), buffers);
    }
    X_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
    glViewport(0, 0, m_specification.m_width, m_specification.m_height);
}

void OpenGLFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
{
    // 校验
    if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
    {
        X_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
        return;
    }
    m_specification.m_width  = width;
    m_specification.m_height = height;
    Invalidate();
}

int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
{
    X_CORE_ASSERT(attachmentIndex < m_colorAttachments.size(), "Invalid attachmentIndex");

    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
    int pixelData;
    glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
    return pixelData;
}

uint32_t OpenGLFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
{
    X_CORE_ASSERT(index < m_colorAttachments.size(), "Out of bounds access");
    return m_colorAttachments[index];
}
