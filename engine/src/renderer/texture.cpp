//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/texture.h"

#include "platform/opengl/opengl_texture.h"
#include "x/core/base.h"
#include "x/core/log.h"
#include "x/renderer/renderer.h"

Ref<Texture2D> Texture2D::Create(const std::string& path) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateRef<OpenGLTexture2D>(path);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}

Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateRef<OpenGLTexture2D>(width, height);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
