//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f), m_squareColor({0.2f, 0.3f, 0.8f, 1.0f})
{
}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
    X_PROFILE_FUNCTION();
    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");

    FramebufferSpecification fbSpec;
    fbSpec.width  = 1280;
    fbSpec.height = 720;
    m_framebuffer = FrameBuffer::Create(fbSpec);
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
    Renderer2D::ResetStats();
    {
        X_PROFILE_SCOPE("Renderer Prep");
        m_framebuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
    }

    {
        X_PROFILE_SCOPE("Renderer Draw");
        static float rotation = 0.0f;
        rotation += ts * 50.0f;
        Renderer2D::BeginScene(m_cameraController.get_camera());
        Renderer2D::DrawRotatedQuad({1.0f, 0.0f}, {0.8f, 0.8f}, -45.0f, {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f, 0.8f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({0.5f, -0.5f}, {0.5f, 0.75f}, m_squareColor);
        Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {20.0f, 20.0f}, m_checkerboardTexture, 10.0f);
        Renderer2D::DrawRotatedQuad({-2.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, rotation, m_checkerboardTexture, 20.0f);
        Renderer2D::EndScene();
        m_framebuffer->Unbind();

        Renderer2D::BeginScene(m_cameraController.get_camera());
        for (float y = -5.0f; y < 5.0f; y += 0.5f)
        {
            for (float x = -5.0f; x < 5.0f; x += 0.5f)
            {
                glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f};
                Renderer2D::DrawQuad({x, y}, {0.45f, 0.45f}, color);
            }
        }
    }
}

void Sandbox2D::OnImguiRender()
{
    X_PROFILE_FUNCTION();
    ImGui::Begin("Settings");
    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    uint32_t textureID = m_checkerboardTexture->GetRendererID();
    ImGui::Image(reinterpret_cast<void *>((uintptr_t)textureID), ImVec2{1280.0f, 720.0f});
    ImGui::End();
}

void Sandbox2D::OnEvent(Event &e)
{
    m_cameraController.OnEvent(e);
}
