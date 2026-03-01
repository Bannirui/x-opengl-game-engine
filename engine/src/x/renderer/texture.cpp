//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/texture.h"

#include "platform/opengl/open_gl_texture.h"
#include "x/renderer/renderer.h"
#include "x/core/x_log.h"
#include "x/core/core.h"

X::Ref<Texture2D> Texture2D::Create(const std::string &path)
{
    switch (Renderer::GetAPI())
    {
        case RendererAPI::API::kNone:
        {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL:
        {
            return X::CreateRef<OpenGLTexture2D>(path);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

X::Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
{
    switch (Renderer::GetAPI())
    {
        case RendererAPI::API::kNone:
        {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL:
        {
            return X::CreateRef<OpenGLTexture2D>(width, height);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
