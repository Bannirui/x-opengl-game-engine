//
// Created by dingrui on 3/7/26.
//

#include "platform//opengl/opengl_uniform_buffer.h"

#include <glad/glad.h>

OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
{
    // 生成GPU Buffer对象
    glGenBuffers(1, &m_rendererID);
    // 绑定Buffer类型 UBO类型
    glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
    // 分配GPU显存 只是先分配空内存 暂时没有CPU内存的数据传过去
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    /**
     * UBO的关键 绑定到binding port 跟shader对应
     *   - 这里binding=0 -> GPU会自动slot=0 -> Camera uniform -> shader渲染
     *   - 这里binding=1 -> GPU会自动slot=1 -> Lights uniform -> shader渲染
     */
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_rendererID);
    // 解绑OpenGL状态机 防止后面误操作
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
    // 准备给GPU显存的EBO灌数据 先告诉OpenGL状态机槽位绑定到这个UBO
    glBindBuffer(GL_UNIFORM_BUFFER, m_rendererID);
    // 把内存地址data偏移offset的位置 size个字节数据灌给显存
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    // 操作完UBO让OpenGL状态机槽位解绑 防止后面其他人误操作
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
