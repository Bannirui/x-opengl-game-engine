//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/buffer.h"

#include "platform/opengl/open_gl_buffer.h"
#include "x/renderer/renderer.h"
#include "x/core/core.h"
#include "x/core/x_log.h"

X::Ref<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size)
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
            return X::CreateRef<OpenGLVertexBuffer>(vertices, size);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

X::Ref<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t count)
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
            return X::CreateRef<OpenGLIndexBuffer>(indices, count);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
