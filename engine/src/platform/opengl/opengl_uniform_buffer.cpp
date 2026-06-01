//
// Created by dingrui on 3/7/26.
//

#include "platform//opengl/opengl_uniform_buffer.h"

#include <glad/glad.h>

OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
    : m_binding(binding)
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

/**
 * 把CPU内存上的数据灌给UBO 也就是把数据从CPU内存写到GPU显存
 * [x...offset...y)比如内存地址x上 从offset开始读 offset到y总共n个字节的数据
 * 把这些数据读出来写给显存
 * @param data CPU内存上的数据地址
 * @param size CPU内存上多大的数据 多少个字节
 * @param offset 从这个CPU内存地址什么地方开始读
 */
void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLUniformBuffer::Bind() const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, m_binding, m_rendererID);
}
