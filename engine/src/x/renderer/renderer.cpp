//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer.h"

#include "x/renderer/vertex_array.h"
#include "x/renderer/render_command.h"
#include "x/renderer/orthographic_camera.h"
#include "x/renderer/shader.h"

Renderer::SceneData *Renderer::s_sceneData = new Renderer::SceneData;

void Renderer::BeginScene(OrthographicCamera &camera) {
    s_sceneData->viewProjectionMatrix = camera.get_viewProjectionMatrix();
}

void Renderer::EndScene() {
}

void Renderer::Submit(const std::shared_ptr<Shader> &shader, const std::shared_ptr<VertexArray> &vertexArray) {
    shader->Bind();
    shader->UploadUniformMat4("u_ViewProjection", s_sceneData->viewProjectionMatrix);

    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
}
