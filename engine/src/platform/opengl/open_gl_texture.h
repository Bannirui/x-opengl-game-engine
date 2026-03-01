//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/renderer/texture.h"

// mac对OpenGL的支持只到3.3 只支持先绑定再操作 不支持DSA(不绑定直接操作) 所以统一用传统方式先绑定再操作
class OpenGLTexture2D : public Texture2D
{
public:
    OpenGLTexture2D(const std::string &path);
    OpenGLTexture2D(uint32_t width, uint32_t height);
    ~OpenGLTexture2D() override;

    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }

    void SetData(void *data, uint32_t size) override;

    void Bind(uint32_t slot = 0) const override;

private:
    std::string m_path;
    uint32_t    m_width;
    uint32_t    m_height;
    uint32_t    m_rendererId;
    uint32_t    m_internalFormat, m_dataFormat;
};
