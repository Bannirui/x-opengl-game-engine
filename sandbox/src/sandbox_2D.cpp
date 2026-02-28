//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

#include "platform/opengl/open_gl_shader.h"

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) {
}

Sandbox2D::~Sandbox2D() {
}

void Sandbox2D::OnAttach() {
}

void Sandbox2D::OnDetach() {
}

void Sandbox2D::OnUpdate(Timestep ts) {
    // update
    m_cameraController.OnUpdate(ts);
    // render
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    RenderCommand::Clear();

    Renderer2D::BeginScene(m_cameraController.get_camera());
    Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {0.8f, 0.2f, 0.3f, 1.0f});
    Renderer2D::EndScene();
}

void Sandbox2D::OnImguiRender() {
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Event &e) {
    m_cameraController.OnEvent(e);
}
