#include "x/renderer/graphics_context.h"

#include "platform/opengl/opengl_context.h"
#include "x/core/base.h"
#include "x/core/log.h"
#include "x/renderer/renderer.h"

X::Scope<GraphicsContext> GraphicsContext::Create(void* window) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return X::CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
