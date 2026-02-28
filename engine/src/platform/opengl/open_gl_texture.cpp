//
// Created by dingrui on 2/28/26.
//

#include "platform/opengl/open_gl_texture.h"

#include <stb_image.h>
#include <glad/glad.h>

#include "x/core.h"
#include "x/x_log.h"

OpenGLTexture2D::OpenGLTexture2D(const std::string &path)
    : m_path(path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    X_CORE_ASSERT(data, "Failed to load image");
    m_width = width;
    m_height = height;

    // 生成纹理对象
    glGenTextures(1, &m_rendererId);
    // 激活纹理对象
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_rendererId);

    // 分配cpu内存
    GLenum internalFormat = GL_RGB8;
    GLenum format = GL_RGB;
    if (channels == 4)
    {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    // 设置纹理对象参数
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 解绑纹理对象
    glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    if (m_rendererId) {
        glDeleteTextures(1, &m_rendererId);
    }
}

void OpenGLTexture2D::Bind(uint32_t slot) const {
    // OpenGL是状态机 不知道当前状态机的纹理单元 所以要先切换纹理单元 然后绑定纹理对象
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_rendererId);
}
