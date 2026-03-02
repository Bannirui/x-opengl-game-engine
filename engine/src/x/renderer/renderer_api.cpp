//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer_api.h"

#include "x/renderer/renderer.h"
#include "x/core/base.h"
#include "x/core/x_log.h"
#include "platform/opengl/open_gl_renderer_api.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::kOpenGL;

X::Scope<RendererAPI> RendererAPI::Create()
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
            return X::CreateScope<OpenGLRendererAPI>();
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
