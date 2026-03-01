//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
    X_PROFILE_FUNCTION();
    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
    X_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Timestep ts)
{
    X_PROFILE_FUNCTION();
    // update
    m_cameraController.OnUpdate(ts);

    // render
    {
        X_PROFILE_SCOPE("Renderer Prep");
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
    }

    {
        X_PROFILE_SCOPE("Renderer Draw");
        Renderer2D::BeginScene(m_cameraController.get_camera());
        Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({0.5, -0.5f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
        Renderer2D::DrawQuad({0.0f, 0.0f, -1.0f}, {10.0f, 10.0f}, m_checkerboardTexture);
        Renderer2D::EndScene();
    }
}

void Sandbox2D::OnImguiRender()
{
    X_PROFILE_FUNCTION();
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Event &e)
{
    m_cameraController.OnEvent(e);
}
