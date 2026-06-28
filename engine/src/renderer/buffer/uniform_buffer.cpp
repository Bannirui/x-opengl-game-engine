//
// Created by dingrui on 3/7/26.
//

#include "x/renderer/buffer/uniform_buffer.h"

#include "platform/opengl/opengl_uniform_buffer.h"
#include "x/renderer/renderer.h"

Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateRef<OpenGLUniformBuffer>(size, binding);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
