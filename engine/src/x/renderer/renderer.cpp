//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer.h"

#include "x/renderer/renderer_2D.h"
#include "x/renderer/vertex_array.h"
#include "x/renderer/render_command.h"
#include "x/renderer/orthographic_camera.h"
#include "x/renderer/shader.h"
#include "platform/opengl/open_gl_shader.h"

X::Scope<Renderer::SceneData> Renderer::s_sceneData = X::CreateScope<Renderer::SceneData>();

void Renderer::Init() {
    RenderCommand::Init();
    Renderer2D::Init();
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
    RenderCommand::SetViewport(0, 0, width, height);
}

void Renderer::BeginScene(OrthographicCamera &camera) {
    s_sceneData->viewProjectionMatrix = camera.get_viewProjectionMatrix();
}

void Renderer::EndScene() {
}

void Renderer::Submit(const std::shared_ptr<Shader> &shader,
                      const std::shared_ptr<VertexArray> &vertexArray,
                      const glm::mat4 &transform) {
    shader->Bind();
    std::dynamic_pointer_cast<OpenGLShader>(shader)->SetMat4("u_ViewProjection", s_sceneData->viewProjectionMatrix);
    std::dynamic_pointer_cast<OpenGLShader>(shader)->SetMat4("u_Transform", transform);

    vertexArray->Bind();
    RenderCommand::DrawIndexed(vertexArray);
}
