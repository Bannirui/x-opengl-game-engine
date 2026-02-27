//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/buffer.h"

#include "platform/opengl/open_gl_buffer.h"
#include "x/renderer/renderer.h"
#include "x/core.h"
#include "x/x_log.h"

VertexBuffer *VertexBuffer::Create(float *vertices, uint32_t size)
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
            return new OpenGLVertexBuffer(vertices, size);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

/**
 * @param indices index array
 * @param count how many vertex
 * @return
 */
IndexBuffer *IndexBuffer::Create(uint32_t *indices, uint32_t count)
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
            return new OpenGLIndexBuffer(indices, count);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
