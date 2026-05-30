//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <glm/gtc/type_ptr.inl>

#include <imgui.h>

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f), m_squareColor({0.2f, 0.3f, 0.8f, 1.0f}) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach() {
    X_PROFILE_FUNCTION();
    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");
    m_cppTexture = Texture2D::Create("asset/texture/CPPLogo.png");
}

void Sandbox2D::OnDetach() {
    X_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Timestep ts) {
    X_PROFILE_FUNCTION();
    // update
    m_cameraController.OnUpdate(ts);

    // render
    Renderer2D::ResetStats();
    {
        X_PROFILE_SCOPE("Renderer Prep");
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
    }

    {
        X_PROFILE_SCOPE("Renderer Draw");
        static float rotation = 0.0f;
        rotation += ts * 50.0f;

        Renderer2D::BeginScene(m_cameraController.get_camera());

        // 普通矩形
        Renderer2D::DrawQuad({-1.5f, -0.5f}, {1.0f, 1.0f}, {0.2f, 0.3f, 0.8f, 1.0f});
        // 纹理矩形
        Renderer2D::DrawQuad({0.0f, -0.5f}, {1.0f, 1.0f}, m_checkerboardTexture);
        // 旋转矩形
        Renderer2D::DrawRotatedQuad({2.5f, -1.0f}, /*右移2.5 下移0.5*/
            {2.0f, 0.5f},/*长2倍 宽一半*/
            rotation,/*旋转*/
            m_cppTexture,/*贴图*/
            10.0f/*亮度调节*/
            );
        // 圆形
        {
            // 变换矩阵
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), {-1.5f, 1.5f, 0.0f}) * /*再平移*/
                                  glm::scale(glm::mat4(1.0f), {1.0f, 1.0f, 1.0f});       /*先缩放*/
            Renderer2D::DrawCircle(transform, {0.8f, 0.2f, 0.3f, 1.0f}, 1.0f, 0.01f);
        }
        // 矩形框
        Renderer2D::DrawRect({0.0f, 1.5f, 0.0f}, {1.0f, 1.0f}, {0.2f, 0.8f, 0.2f, 1.0f});
        // 旋转的线
        glm::vec3 p0 = {1.5f, 1.0f, 0.0f};
        float r = glm::radians(rotation);
        glm::vec3 p1 = {1.5f + cos(r) * 0.5f, 1.0f + sin(r) * 0.5f, 0.0f};
        Renderer2D::DrawLine(p0, p1, {0.8f, 0.8f, 0.2f, 1.0f});
        // 所有图形的VBO数据一起提交给GPU
        Renderer2D::EndScene();
    }
}

void Sandbox2D::OnImguiRender() {
    X_PROFILE_FUNCTION();
    ImGui::Begin("Settings");
    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    // 有多少个图形是用4个顶点画2个三角形生成的
    ImGui::Text("Primitives: %d", stats.PrimitiveCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Event& e) {
    m_cameraController.OnEvent(e);
}
