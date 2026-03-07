//
// Created by dingrui on 3/7/26.
//

#include "platform//opengl/open_gl_uniform_buffer.h"

#include <glad/glad.h>

OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
{
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_rendererID);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

OpenGLUniformBuffer::~OpenGLUniformBuffer()
{
    if (m_rendererID != 0)
    {
        glDeleteBuffers(1, &m_rendererID);
    }
}

void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
