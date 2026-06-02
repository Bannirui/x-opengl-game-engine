//
// Created by rui ding on 2026/6/1.
//

#include "sandbox_3D.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

// ---- Procedural cube ----
static X::Ref<Mesh> CreateCubeMesh() {
    std::vector<StaticMeshVertex> vertices = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        // Back face
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        // Top face
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        // Right face
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        // Left face
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    };
    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,   // Front
        4,  5,  6,  6,  7,  4,   // Back
        8,  9,  10, 10, 11, 8,   // Top
        12, 13, 14, 14, 15, 12,  // Bottom
        16, 17, 18, 18, 19, 16,  // Right
        20, 21, 22, 22, 23, 20,  // Left
    };
    return Mesh::Create(vertices, indices);
}

// ---- Procedural plane ----
static X::Ref<Mesh> CreatePlaneMesh() {
    std::vector<StaticMeshVertex> vertices = {
        {{-5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{5.0f, -0.5f, -5.0f}, {0.0f, 1.0f, 0.0f}, {10.0f, 0.0f}},
        {{5.0f, -0.5f, 5.0f}, {0.0f, 1.0f, 0.0f}, {10.0f, 10.0f}},
        {{-5.0f, -0.5f, 5.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 10.0f}},
    };
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    return Mesh::Create(vertices, indices);
}

Sandbox3D::Sandbox3D()
    : Layer("Sandbox3D"), m_editorCamera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f) {
    m_editorCamera.set_distance(8.0f);
}

Sandbox3D::~Sandbox3D() {}

void Sandbox3D::OnAttach() {
    X_PROFILE_FUNCTION();

    // Create cube mesh and material
    m_cubeMesh = CreateCubeMesh();
    m_planeMesh = CreatePlaneMesh();

    // Load PBR shader
    m_pbrShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");

    // Dummy white texture for missing PBR maps
    X::Ref<Texture2D> whiteTex = Texture2D::Create(1, 1);
    uint32_t whiteData = 0xffffffff;
    whiteTex->SetData(&whiteData, sizeof(uint32_t));

    // Cube material (metallic + roughness)
    m_cubeMaterial = Material::Create(m_pbrShader);
    m_cubeMaterial->SetFloat3("u_Albedo", glm::vec3(0.8f, 0.3f, 0.3f));
    m_cubeMaterial->SetFloat("u_Metallic", 0.2f);
    m_cubeMaterial->SetFloat("u_Roughness", 0.3f);
    m_cubeMaterial->SetFloat("u_AO", 1.0f);
    m_cubeMaterial->SetTexture("u_AlbedoMap", whiteTex);
    m_cubeMaterial->SetTexture("u_MetallicMap", whiteTex);
    m_cubeMaterial->SetTexture("u_RoughnessMap", whiteTex);
    m_cubeMaterial->SetTexture("u_AOMap", whiteTex);

    // Plane material
    m_planeMaterial = Material::Create(m_pbrShader);
    m_planeMaterial->SetFloat3("u_Albedo", glm::vec3(0.5f));
    m_planeMaterial->SetFloat("u_Metallic", 0.0f);
    m_planeMaterial->SetFloat("u_Roughness", 0.8f);
    m_planeMaterial->SetFloat("u_AO", 1.0f);
    m_planeMaterial->SetTexture("u_AlbedoMap", whiteTex);
    m_planeMaterial->SetTexture("u_MetallicMap", whiteTex);
    m_planeMaterial->SetTexture("u_RoughnessMap", whiteTex);
    m_planeMaterial->SetTexture("u_AOMap", whiteTex);
}

void Sandbox3D::OnDetach() {
    X_PROFILE_FUNCTION();
    m_cubeMesh.reset();
    m_planeMesh.reset();
    m_cubeMaterial.reset();
    m_planeMaterial.reset();
    m_pbrShader.reset();
    m_envMap.reset();
}

void Sandbox3D::OnUpdate(Timestep ts) {
    X_PROFILE_FUNCTION();

    // Update camera
    m_editorCamera.OnUpdate(ts);

    // Rotation
    m_rotation += ts * 30.0f;

    // Render
    Renderer3D::ResetStats();
    RenderCommand::SetClearColor({0.05f, 0.05f, 0.1f, 1.0f});
    RenderCommand::Clear();

    Renderer3D::SetLightDirection(m_lightDirection);
    Renderer3D::SetLightColor(m_lightColor);
    Renderer3D::SetExposure(m_exposure);

    if (m_envMap) {
        Renderer3D::DrawSkybox();
    }

    Renderer3D::BeginScene(m_editorCamera);

    // Rotating cube
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)) *
                              glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
        Renderer3D::DrawMesh(m_cubeMesh, m_cubeMaterial, transform);
    }

    // Second cube (smaller, orbiting)
    {
        float orbitRadius = 2.5f;
        float x = glm::cos(glm::radians(m_rotation * 1.5f)) * orbitRadius;
        float z = glm::sin(glm::radians(m_rotation * 1.5f)) * orbitRadius;
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, 1.5f, z)) *
                              glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation * 2.0f),
                                          glm::vec3(0.3f, 0.7f, 0.2f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(0.7f));
        Renderer3D::DrawMesh(m_cubeMesh, m_cubeMaterial, transform);
    }

    // Ground plane
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
        Renderer3D::DrawMesh(m_planeMesh, m_planeMaterial, transform);
    }

    Renderer3D::EndScene();
}

void Sandbox3D::OnImguiRender() {
    X_PROFILE_FUNCTION();

    ImGui::Begin("3D Scene Settings");
    auto stats = Renderer3D::GetStats();
    ImGui::Text("Renderer3D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Meshes: %d", stats.MeshCount);

    ImGui::Separator();
    ImGui::DragFloat3("Light Direction", glm::value_ptr(m_lightDirection), 0.05f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_lightColor));
    ImGui::DragFloat("Exposure", &m_exposure, 0.05f, 0.1f, 5.0f);
    ImGui::End();
}

void Sandbox3D::OnEvent(Event& e) {
    m_editorCamera.OnEvent(e);
}
