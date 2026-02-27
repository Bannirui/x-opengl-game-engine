//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/vertex_array.h"

#include "renderer_api.h"
#include "renderer_api.h"
#include "platform/opengl/open_gl_vertex_array.h"
#include "x/renderer/renderer.h"
#include "x/renderer/renderer_api.h"
#include "x/core.h"
#include "x/x_log.h"

VertexArray *VertexArray::Create()
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
            return new OpenGLVertexArray();
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
