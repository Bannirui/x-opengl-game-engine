//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/opengl_buffer.h"

#include <glad/glad.h>

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count) : m_count(count) {
    X_PROFILE_FUNCTION();
    // OpenGL在显存上申请块空间 用唯一id引用它
    glGenBuffers(1, &m_rendererID);
    // 激活OpenGL的Array_Buffer插槽 准备发数据到显存
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    // 索引值都是整数 很容易计算出来要往显存发送多少字节的内容
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
    X_PROFILE_FUNCTION();
    glDeleteBuffers(1, &m_rendererID);
}

void OpenGLIndexBuffer::Bind() const {
    X_PROFILE_FUNCTION();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void OpenGLIndexBuffer::Unbind() const {
    X_PROFILE_FUNCTION();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
