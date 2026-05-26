//
// Created by dingrui on 3/7/26.
//

#include "x/renderer/uniform_buffer.h"

#include "platform/opengl/open_gl_uniform_buffer.h"
#include "x/renderer/renderer.h"

X::Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
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
            return X::CreateRef<OpenGLUniformBuffer>(size, binding);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
