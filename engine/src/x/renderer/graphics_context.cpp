#include "x/renderer/graphics_context.h"

#include "platform/opengl/open_gl_context.h"
#include "x/renderer/renderer.h"
#include "x/core/base.h"
#include "x/core/x_log.h"

X::Scope<GraphicsContext> GraphicsContext::Create(void *window)
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
            return X::CreateScope<OpenGLContext>(static_cast<GLFWwindow *>(window));
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
