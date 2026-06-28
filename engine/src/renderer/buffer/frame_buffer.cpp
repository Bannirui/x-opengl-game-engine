//
// Created by dingrui on 3/3/26.
//

#include "x/renderer/buffer/frame_buffer.h"

#include "platform/opengl/opengl_framebuffer.h"
#include "x/renderer/renderer.h"

Ref<FrameBuffer> FrameBuffer::Create(const FramebufferSpecification& spec) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateRef<OpenGLFramebuffer>(spec);
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
