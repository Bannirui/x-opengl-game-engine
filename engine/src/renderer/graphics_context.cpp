#include "x/renderer/graphics_context.h"

#include "platform/opengl/opengl_context.h"
#include "x/core/base.h"
#include "x/log/log.h"
#include "x/renderer/renderer.h"

Scope<GraphicsContext> GraphicsContext::Create(void* window) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
        }
    }
    CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
