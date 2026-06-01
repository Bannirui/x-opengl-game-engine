//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/opengl_buffer.h"

#include <glad/glad.h>

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/**
 * 创建VBO(vertex buffer object)
 * GPU显存上开辟连续的空间用来放顶点数据
 * @param vertices VBO要放的顶点数据 在内存上的地址
 * @param size 要在显存上开辟多大空间 多少个字节
 */
OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t size)
{
    X_PROFILE_FUNCTION();
    // 从显存申请个buffer object 就是VBO
    glGenBuffers(1, &m_rendererID);
    // 绑定buffer类型 VBO的类型是array buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    /**
     * 把内存数据拷贝到显存
     *   - 参数1 显存buffer类型 VBO绑定的是array buffer
     *   - 参数2 要传多大的数据 字节
     *   - 参数3 要传的数据 就是数据的内存地址
     *   - 参数4 希望GPU怎么管理这些数据
     *     - GL_STREAM_DRAW 数据只会传一次 GPU用的次数也少
     *     - GL_STATIC_DRAW 数据只会传一次 但是GPU要经常使用
     *     - GL_DYNAMIC_DRAW 数据要反复多次传过去 GPU也会高频使用
     */
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
{
    X_PROFILE_FUNCTION();
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    X_PROFILE_FUNCTION();
    glDeleteBuffers(1, &m_rendererID);
}

void OpenGLVertexBuffer::Bind() const
{
    X_PROFILE_FUNCTION();
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void OpenGLVertexBuffer::Unbind() const
{
    X_PROFILE_FUNCTION();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::SetData(const void *data, uint32_t size)
{
    X_PROFILE_FUNCTION();

    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t count) : m_count(count)
{
    X_PROFILE_FUNCTION();
    // OpenGL在显存上申请块空间 用唯一id引用它
    glGenBuffers(1, &m_rendererID);
    // 激活OpenGL的Array_Buffer插槽 准备发数据到显存
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    // 索引值都是整数 很容易计算出来要往显存发送多少字节的内容
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
    X_PROFILE_FUNCTION();
    glDeleteBuffers(1, &m_rendererID);
}

void OpenGLIndexBuffer::Bind() const
{
    X_PROFILE_FUNCTION();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void OpenGLIndexBuffer::Unbind() const
{
    X_PROFILE_FUNCTION();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
