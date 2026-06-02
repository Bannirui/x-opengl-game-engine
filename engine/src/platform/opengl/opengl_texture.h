//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/renderer/texture.h"

// mac对OpenGL的支持只到3.3 只支持先绑定再操作 不支持DSA(不绑定直接操作) 所以统一用传统方式先绑定再操作
class OpenGLTexture2D : public Texture2D {
public:
    /**
     * 创建贴图 用的磁盘上现有的图片
     * @param path 图片路径
     */
    OpenGLTexture2D(const std::string& path);
    OpenGLTexture2D(uint32_t width, uint32_t height);
    ~OpenGLTexture2D() override;

    uint32_t GetWidth() const override {
        return m_width;
    }

    uint32_t GetHeight() const override {
        return m_height;
    }

    uint32_t GetRendererID() const override {
        return m_rendererId;
    }

    const std::string& GetPath() const override {
        return m_path;
    }

    void SetData(void* data, uint32_t size) override;

    void Bind(uint32_t slot = 0) const override;

    bool IsLoaded() override {
        return m_isLoaded;
    }

    bool operator==(const Texture& other) const override {
        return m_rendererId == other.GetRendererID();
    }

private:
    // 跟VBO VAO这些object一样 texture也用分配的唯一id引用
    uint32_t m_rendererId{0};
    // 纹理对象用的贴图 图片路径
    std::string m_path;
    // 纹理对象图片的宽度
    uint32_t m_width;
    // 纹理对象图片的高度
    uint32_t m_height;
    uint32_t m_internalFormat, m_dataFormat;
    bool m_isLoaded{false};
};
