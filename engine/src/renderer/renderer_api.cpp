//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer_api.h"

#include "platform/opengl/opengl_renderer_api.h"
#include "x/core/base.h"
#include "x/log/log.h"
#include "x/renderer/renderer.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::kOpenGL;

Scope<RendererAPI> RendererAPI::Create() {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateScope<OpenGLRendererAPI>();
        }
    }
    CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
