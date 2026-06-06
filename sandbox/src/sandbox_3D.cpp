//
// Created by rui ding on 2026/6/6.
//

#include "sandbox_3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <x/renderer/3d/renderer_3D.h>
#include <x/renderer/camera/camera.h>
#include <x/renderer/material.h>
#include <x/renderer/mesh.h>
#include <x/renderer/render_command.h>
#include <x/renderer/shader.h>
#include <x/renderer/texture.h>

#include <imgui.h>

Sandbox3D::Sandbox3D() : Layer("Sandbox3D") {}

void Sandbox3D::OnAttach() {
    X_PROFILE_FUNCTION();

    m_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    InitLevel3PBR();
}

void Sandbox3D::OnDetach() {
    X_PROFILE_FUNCTION();
    m_pbrShader.reset();
    m_cubeMesh.reset();
    m_cubeMaterial.reset();
}

void Sandbox3D::InitLevel3PBR() {
    // clang-format off
    std::vector<StaticMeshVertex> vertices = {
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    };
    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };
    // clang-format on

    m_cubeMesh = Mesh::Create(vertices, indices);

    m_pbrShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");

    X::Ref<Texture2D> whiteTex = Texture2D::Create(1, 1);
    uint32_t whiteData = 0xffffffff;
    whiteTex->SetData(&whiteData, sizeof(uint32_t));

    m_cubeMaterial = Material::Create(m_pbrShader);
    m_cubeMaterial->SetFloat3("u_Albedo", glm::vec3(0.8f, 0.3f, 0.3f));
    m_cubeMaterial->SetFloat("u_Metallic", 0.0f);
    m_cubeMaterial->SetFloat("u_Roughness", 0.5f);
    m_cubeMaterial->SetFloat("u_AO", 1.0f);
    m_cubeMaterial->SetTexture("u_AlbedoMap", whiteTex);
    m_cubeMaterial->SetTexture("u_MetallicMap", whiteTex);
    m_cubeMaterial->SetTexture("u_RoughnessMap", whiteTex);
    m_cubeMaterial->SetTexture("u_AOMap", whiteTex);
}

// ============================================================
// OnUpdate
// ============================================================
void Sandbox3D::OnUpdate(Timestep ts) {
    X_PROFILE_FUNCTION();

    m_rotation += ts * 30.0f;

    RenderCommand::SetClearColor({0.1f, 0.2f, 0.3f, 1.0f});
    RenderCommand::Clear();

    Renderer3D::ResetStats();

    Renderer3D::SetLightDirection({-0.5f, -1.0f, -0.3f});
    Renderer3D::SetLightColor({1.0f, 0.95f, 0.9f});
    Renderer3D::SetExposure(1.2f);

    // 用简单的透视相机传入 BeginScene
    Camera simpleCamera(m_projectionMatrix);

    Renderer3D::BeginScene(simpleCamera, m_viewMatrix);
    {
        glm::mat4 M = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        Renderer3D::DrawMesh(m_cubeMesh, m_cubeMaterial, M);
    }
    Renderer3D::EndScene();
}

void Sandbox3D::OnImguiRender() {
    X_PROFILE_FUNCTION();

    ImGui::Begin("3D");

    ImGui::Separator();

    ImGui::Text("Level 0: only view color: glClear ok");
    ImGui::Text("Level 1: triangle: Draw ok");
    ImGui::Text("Level 2: cube: IBO/EBO ok");
    ImGui::Text("Level 3: PBR: Renderer3D ok");

    ImGui::Separator();
    ImGui::Text("rotate: %.1f", m_rotation);

    auto stats = Renderer3D::GetStats();
    ImGui::Text("Renderer3D Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Renderer3D Meshes: %d", stats.MeshCount);

    ImGui::End();
}

void Sandbox3D::OnEvent(Event& e) {
    // 简单相机: 无输入交互, 固定视角
}
