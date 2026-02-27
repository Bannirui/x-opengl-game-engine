//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer.h"

#include "x/renderer/vertex_array.h"
#include "x/renderer/render_command.h"

void Renderer::BeginScene() {}

void Renderer::EndScene() {}

void Renderer::Submit(const std::shared_ptr<VertexArray> &vertexArray)
{
    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
}
