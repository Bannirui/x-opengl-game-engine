//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "x/renderer/frame_buffer.h"
#include "x/renderer/render_command.h"
#include "x/renderer/renderer_2D.h"
#include "x/renderer/texture.h"

EditorLayer::EditorLayer()
    : Layer("EditorLayer"), m_cameraController(1280.0f / 720.0f), m_squareColor({0.2f, 0.3f, 0.4f, 1.0f})
{
}

void EditorLayer::OnAttach()
{
    X_PROFILE_FUNCTION();
    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");

    FramebufferSpecification fbSpec;
    fbSpec.width  = 1280;
    fbSpec.height = 720;
    m_frameBuffer = FrameBuffer::Create(fbSpec);
}

void EditorLayer::OnDetach()
{
    X_PROFILE_FUNCTION();
}

void EditorLayer::OnUpdate(Timestep ts)
{
    X_PROFILE_FUNCTION();

    // update
    m_cameraController.OnUpdate(ts);
    // render
    Renderer2D::ResetStats();
    {
        X_PROFILE_SCOPE("Renderer Prep");
        m_frameBuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.2f, 0.3f, 1.0f});
        RenderCommand::Clear();
    }
    {
        static float rotation = 0.0f;
        rotation += ts * 50.0f;
        X_PROFILE_SCOPE("Renderer Draw");
        Renderer2D::BeginScene(m_cameraController.get_camera());
        Renderer2D::DrawRotatedQuad({1.0f, 0.0f}, {0.8f, 0.8f}, -45.0f, {0.8f, 0.2, 0.3, 1.0f});
        Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f, 0.8f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({0.5f, -0.5f}, {0.5f, 0.75f}, m_squareColor);
        Renderer2D::DrawQuad({0.0f, 0.0f, -0.1f}, {20.0f, 20.0f}, m_checkerboardTexture, 10.0f);
        Renderer2D::DrawRotatedQuad({0.0f, 0.0f, -0.1f}, {20.0f, 20.0f}, rotation, m_checkerboardTexture, 20.0f);
        Renderer2D::EndScene();

        Renderer2D::BeginScene(m_cameraController.get_camera());
        for (float y = -5.0f; y < 5.0f; y += 0.5)
        {
            for (float x = -5.0f; x < 5.0f; x += 0.5f)
            {
                glm::vec4 color = {(x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f};
                Renderer2D::DrawQuad({x, y}, {0.45f, 0.45f}, color);
            }
        }
        Renderer2D::EndScene();
        m_frameBuffer->Unbind();
    }
}

void EditorLayer::OnImguiRender()
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
    ImGui::End();
}

void EditorLayer::OnEvent(Event &e)
{
    m_cameraController.OnEvent(e);
}
