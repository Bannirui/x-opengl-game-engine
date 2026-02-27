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

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererId);
    glTextureStorage2D(m_rendererId, 1, GL_RGB8, m_width, m_height);

    glTextureParameteri(m_rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureSubImage2D(m_rendererId, 0, 0, 0, m_width, m_height,GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    if (m_rendererId) {
        glDeleteTextures(1, &m_rendererId);
    }
}

void OpenGLTexture2D::Bind(uint32_t slot) const {
    glBindTextureUnit(GL_TEXTURE0 + slot, m_rendererId);
}
