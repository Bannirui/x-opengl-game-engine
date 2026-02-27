//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/open_gl_renderer_api.h"

#include <glad/glad.h>

#include "x/renderer/vertex_array.h"
#include "x/renderer/buffer.h"

void OpenGLRendererAPI::SetClearColor(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray)
{
    glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
