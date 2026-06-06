//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/buffer/buffer.h"

#include "platform/opengl/opengl_buffer.h"
#include "x/core/base.h"
#include "x/core/x_log.h"
#include "x/renderer/renderer.h"

X::Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return X::CreateRef<OpenGLVertexBuffer>(vertices, size);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}

X::Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return X::CreateRef<OpenGLVertexBuffer>(size);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}

/**
 * @param indices 顶点索引数组在内存的什么位置 内存地址
 * @param count 多少个顶点的索引
 */
X::Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return X::CreateRef<OpenGLIndexBuffer>(indices, count);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
