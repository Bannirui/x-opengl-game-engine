//
// Created by dingrui on 3/3/26.
//

#include "x/renderer/frame_buffer.h"

#include "platform/opengl/open_gl_framebuffer.h"
#include "x/renderer/renderer.h"

X::Ref<FrameBuffer> FrameBuffer::Create(const FramebufferSpecification &spec)
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
            return X::CreateRef<OpenGLFramebuffer>(spec);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
