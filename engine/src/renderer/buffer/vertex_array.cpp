//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/buffer/vertex_array.h"

#include "platform/opengl/opengl_vertex_array.h"
#include "x/core/log.h"
#include "x/renderer/renderer.h"
#include "x/renderer/renderer_api.h"

/**
 * @return VAO
 */
Ref<VertexArray> VertexArray::Create() {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ERROR("RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return CreateRef<OpenGLVertexArray>();
        }
    }
    X_CORE_ERROR("Unknown RendererAPI!");
    return nullptr;
}
