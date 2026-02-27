//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/renderer/texture.h"

class OpenGLTexture2D : public Texture2D {
public:
    OpenGLTexture2D(const std::string &path);

    ~OpenGLTexture2D() override;

    uint32_t GetWidth() const override { return m_width; }

    uint32_t GetHeight() const override { return m_height; }

    void Bind(uint32_t slot = 0) const override;

private:
    std::string m_path;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_rendererId;
};
