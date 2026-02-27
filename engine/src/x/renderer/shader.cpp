//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/shader.h"

#include "platform/opengl/open_gl_shader.h"
#include "x/renderer/renderer.h"
#include "x/x_log.h"
#include "x/core.h"

Shader *Shader::Create(const std::string &vertexSrc, const std::string &fragmentSrc) {
    switch (Renderer::GetAPI()) {
        case RendererAPI::API::kNone: {
            X_CORE_ASSERT(false, "RendererAPI::kNone is currently not supported!");
            return nullptr;
        }
        case RendererAPI::API::kOpenGL: {
            return new OpenGLShader(vertexSrc, fragmentSrc);
        }
    }
    X_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
