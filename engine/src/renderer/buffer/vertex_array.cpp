//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/buffer/vertex_array.h"

#include "platform/opengl/opengl_vertex_array.h"
#include "x/core/x_log.h"
#include "x/renderer/renderer.h"
#include "x/renderer/renderer_api.h"

/**
 * @return VAO
 */
X::Ref<VertexArray> VertexArray::Create() {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return X::CreateRef<OpenGLVertexArray>();
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
