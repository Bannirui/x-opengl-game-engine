//
// Created by rui ding on 2026/6/6.
//

#include "sandbox_3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include <x/renderer/3d/renderer_3D.h>
#include <x/renderer/buffer/buffer.h>
#include <x/renderer/buffer/vertex_array.h>
#include <x/renderer/camera/camera.h>
#include <x/renderer/material.h>
#include <x/renderer/mesh.h>
#include <x/renderer/render_command.h>
#include <x/renderer/shader.h>
#include <x/renderer/texture.h>

Sandbox3D::Sandbox3D() : Layer("Sandbox3D") {}

void Sandbox3D::OnAttach() {
    X_PROFILE_FUNCTION();

    m_viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    InitLevel1Triangle();
    InitLevel2Cube();
    InitLevel3PBR();
}

void Sandbox3D::OnDetach() {
    X_PROFILE_FUNCTION();
    m_flatColorShader.reset();
    m_triangleVAO.reset();
    m_cubeVAO.reset();
    m_pbrShader.reset();
    m_cubeMesh.reset();
    m_cubeMaterial.reset();
}

// ============================================================
// Level 0: 仅清屏
// ============================================================

// ============================================================
// Level 1: 三角形 (FlatColor shader + 手动 VAO/VBO)
// 完全绕开 Renderer3D, 最底层验证 OpenGL 绘制
// ============================================================
void Sandbox3D::InitLevel1Triangle() {
    m_flatColorShader = Shader::Create("asset/shader/FlatColor.glsl");

    float vertices[] = {
        // position (x, y, z)
        -0.5f, -0.5f, 0.0f,   // 左下
         0.5f, -0.5f, 0.0f,   // 右下
         0.0f,  0.5f, 0.0f,   // 顶部
    };

    uint32_t indices[] = {0, 1, 2};
    m_triangleIndexCount = 3;

    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    BufferLayout layout = {
        {ShaderDataType::kFloat3, "a_Position"},
    };
    vbo->SetLayout(layout);

    auto ibo = IndexBuffer::Create(indices, m_triangleIndexCount);

    m_triangleVAO = VertexArray::Create();
    m_triangleVAO->AddVertexBuffer(vbo);
    m_triangleVAO->SetIndexBuffer(ibo);
}

// ============================================================
// Level 2: 立方体 (FlatColor shader + 手动 VAO/VBO/IBO)
// ============================================================
void Sandbox3D::InitLevel2Cube() {
    // clang-format off
    float vertices[] = {
        // Front face (4 vertices, position only)
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // Back face
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        // Top face
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        // Right face
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        // Left face
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
    uint32_t indices[] = {
         0,  1,  2,  2,  3,  0,  // Front
         4,  5,  6,  6,  7,  4,  // Back
         8,  9, 10, 10, 11,  8,  // Top
        12, 13, 14, 14, 15, 12,  // Bottom
        16, 17, 18, 18, 19, 16,  // Right
        20, 21, 22, 22, 23, 20,  // Left
    };
    // clang-format on
    m_cubeIndexCount = sizeof(indices) / sizeof(uint32_t);

    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    BufferLayout layout = {
        {ShaderDataType::kFloat3, "a_Position"},
    };
    vbo->SetLayout(layout);

    auto ibo = IndexBuffer::Create(indices, m_cubeIndexCount);

    m_cubeVAO = VertexArray::Create();
    m_cubeVAO->AddVertexBuffer(vbo);
    m_cubeVAO->SetIndexBuffer(ibo);
}

// ============================================================
// Level 3: 立方体 (PBR shader + Renderer3D 完整管线)
// ============================================================
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
// 绘制 Level 1: 三角形
// ============================================================
void Sandbox3D::DrawLevel1Triangle() {
    X_CORE_INFO("开始画三角形");
    m_flatColorShader->Bind();
    m_flatColorShader->SetMat4("u_ViewProjection", m_projectionMatrix * m_viewMatrix);
    m_flatColorShader->SetMat4("u_Transform", glm::mat4(1.0f));
    m_flatColorShader->SetFloat4("u_Color", glm::vec4(1.0f, 0.3f, 0.3f, 1.0f));

    RenderCommand::DrawIndexed(m_triangleVAO, m_triangleIndexCount);
}

// ============================================================
// 绘制 Level 2: 立方体
// ============================================================
void Sandbox3D::DrawLevel2Cube() {
    m_flatColorShader->Bind();
    m_flatColorShader->SetMat4("u_ViewProjection", m_projectionMatrix * m_viewMatrix);

    glm::mat4 transform =
        glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    m_flatColorShader->SetMat4("u_Transform", transform);
    m_flatColorShader->SetFloat4("u_Color", glm::vec4(0.3f, 0.8f, 0.3f, 1.0f));

    RenderCommand::DrawIndexed(m_cubeVAO, m_cubeIndexCount);
}

// ============================================================
// 绘制 Level 3: PBR
// ============================================================
void Sandbox3D::DrawLevel3PBR(Timestep ts) {
    Renderer3D::ResetStats();

    Renderer3D::SetLightDirection({-0.5f, -1.0f, -0.3f});
    Renderer3D::SetLightColor({1.0f, 0.95f, 0.9f});
    Renderer3D::SetExposure(1.2f);

    // 用简单的透视相机传入 BeginScene
    Camera simpleCamera(m_projectionMatrix);
    Renderer3D::BeginScene(simpleCamera, m_viewMatrix);
    {
        glm::mat4 M =
            glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        Renderer3D::DrawMesh(m_cubeMesh, m_cubeMaterial, M);
    }
    Renderer3D::EndScene();
}

// ============================================================
// OnUpdate
// ============================================================
void Sandbox3D::OnUpdate(Timestep ts) {
    X_PROFILE_FUNCTION();

    m_rotation += ts * 30.0f;

    RenderCommand::SetClearColor({0.1f, 0.2f, 0.3f, 1.0f});
    RenderCommand::Clear();

    switch (m_testLevel) {
        case 0:
            break;  // 仅清屏, 什么都不画
        case 1:
            DrawLevel1Triangle();
            break;
        case 2:
            DrawLevel2Cube();
            break;
        case 3:
            DrawLevel3PBR(ts);
            break;
    }
}

void Sandbox3D::OnImguiRender() {
    X_PROFILE_FUNCTION();

    ImGui::Begin("3D");

    ImGui::Text("current level:");
    ImGui::RadioButton("Level 0 - clear", &m_testLevel, 0);
    ImGui::RadioButton("Level 1 - triangle (FlatColor Draw)", &m_testLevel, 1);
    ImGui::RadioButton("Level 2 - cube (FlatColor Draw)", &m_testLevel, 2);
    ImGui::RadioButton("Level 3 - pbr (PBR + Renderer3D)", &m_testLevel, 3);
    X_CORE_INFO("level={}", m_testLevel);

    ImGui::Separator();

    ImGui::Text("Level 0: only view color: glClear ok");
    ImGui::Text("Level 1: triangle: Draw ok");
    ImGui::Text("Level 2: cube: IBO/EBO ok");
    ImGui::Text("Level 3: PBR: Renderer3D ok");

    ImGui::Separator();
    ImGui::Text("rotate: %.1f", m_rotation);

    if (m_testLevel == 3) {
        auto stats = Renderer3D::GetStats();
        ImGui::Text("Renderer3D Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Renderer3D Meshes: %d", stats.MeshCount);
    }

    ImGui::End();
}

void Sandbox3D::OnEvent(Event& e) {
    // 简单相机: 无输入交互, 固定视角
}
