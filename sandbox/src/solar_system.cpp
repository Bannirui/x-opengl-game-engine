//
// Created by rui ding on 2026/6/7.
//

#include "solar_system.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <x/renderer/3d/generator.h>
#include <x/renderer/3d/renderer_3D.h>
#include <x/renderer/render_command.h>
#include <x/renderer/shader.h>
#include <x/renderer/texture.h>

#include <imgui.h>

SolarSystem::SolarSystem() : Layer("SolarSystem"), m_camera(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f) {
    m_camera.set_distance(35.0f);

    m_sun = {.Name = "sun",
             .Albedo = {1.0f, 0.85f, 0.2f},
             .Size = 2.5f,
             .OrbitRadius = 0.0f,
             .OrbitSpeed = 0.0f,
             .RotationSpeed = 10.0f,
             .Metallic = 0.0f,
             .Roughness = 0.3f,
             .Emissive = {1.0f, 0.8f, 0.2f}};

    m_planets = {{
        {.Name = "mercury",
         .Albedo = {0.7f, 0.7f, 0.7f},
         .Size = 0.12f,
         .OrbitRadius = 5.0f,
         .OrbitSpeed = 4.0f,
         .RotationSpeed = 20.0f,
         .Metallic = 0.1f,
         .Roughness = 0.8f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "venus",
         .Albedo = {0.9f, 0.75f, 0.35f},
         .Size = 0.25f,
         .OrbitRadius = 7.5f,
         .OrbitSpeed = 3.0f,
         .RotationSpeed = 15.0f,
         .Metallic = 0.0f,
         .Roughness = 0.7f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "earth",
         .Albedo = {0.2f, 0.4f, 0.8f},
         .Size = 0.27f,
         .OrbitRadius = 10.0f,
         .OrbitSpeed = 2.0f,
         .RotationSpeed = 25.0f,
         .Metallic = 0.0f,
         .Roughness = 0.5f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "mars",
         .Albedo = {0.8f, 0.3f, 0.2f},
         .Size = 0.15f,
         .OrbitRadius = 13.0f,
         .OrbitSpeed = 1.6f,
         .RotationSpeed = 22.0f,
         .Metallic = 0.0f,
         .Roughness = 0.9f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "jupiter",
         .Albedo = {0.7f, 0.5f, 0.3f},
         .Size = 1.0f,
         .OrbitRadius = 17.0f,
         .OrbitSpeed = 0.8f,
         .RotationSpeed = 30.0f,
         .Metallic = 0.0f,
         .Roughness = 0.6f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "saturn",
         .Albedo = {0.85f, 0.75f, 0.5f},
         .Size = 0.85f,
         .OrbitRadius = 21.0f,
         .OrbitSpeed = 0.6f,
         .RotationSpeed = 28.0f,
         .Metallic = 0.0f,
         .Roughness = 0.5f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "uranus",
         .Albedo = {0.3f, 0.7f, 0.8f},
         .Size = 0.5f,
         .OrbitRadius = 25.0f,
         .OrbitSpeed = 0.4f,
         .RotationSpeed = 20.0f,
         .Metallic = 0.0f,
         .Roughness = 0.4f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
        {.Name = "neptune",
         .Albedo = {0.2f, 0.3f, 0.9f},
         .Size = 0.48f,
         .OrbitRadius = 29.0f,
         .OrbitSpeed = 0.3f,
         .RotationSpeed = 18.0f,
         .Metallic = 0.0f,
         .Roughness = 0.4f,
         .Emissive = {0.0f, 0.0f, 0.0f}},
    }};

    m_moon = {.Name = "moon",
              .Albedo = {0.6f, 0.6f, 0.6f},
              .Size = 0.07f,
              .OrbitRadius = 0.6f,
              .OrbitSpeed = 10.0f,
              .RotationSpeed = 5.0f,
              .Metallic = 0.0f,
              .Roughness = 0.9f,
              .Emissive = {0.0f, 0.0f, 0.0f}};
}

void SolarSystem::OnAttach() {
    X_PROFILE_FUNCTION();

    m_sphereMesh = GeometryGenerator::CreateSphere(0.5f, 48, 24);
    m_ringMesh = GeometryGenerator::CreateRing(1.0f - m_orbitThickness, 1.0f + m_orbitThickness, 128);
    m_pbrShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");

    m_whiteTex = Texture2D::Create(1, 1);
    uint32_t whiteData = 0xffffffff;
    m_whiteTex->SetData(&whiteData, sizeof(uint32_t));

    auto loadTex = [this](const std::string& name) -> Ref<Texture2D> {
        std::string path = "asset/texture/" + name + ".jpg";
        Ref<Texture2D> tex = Texture2D::Create(path);
        if (!tex->IsLoaded()) {
            CORE_WARN("Failed to load texture: {}, using fallback", path);
            return m_whiteTex;
        }
        return tex;
    };

    m_sun.AlbedoTexture = loadTex(m_sun.Name);
    for (auto& planet : m_planets) {
        planet.AlbedoTexture = loadTex(planet.Name);
    }
    m_moon.AlbedoTexture = loadTex(m_moon.Name);

    createPlanetMaterial(m_sun);
    for (auto& planet : m_planets) {
        createPlanetMaterial(planet);
    }
    createPlanetMaterial(m_moon);

    m_ringMaterial = Material::Create(m_pbrShader);
    m_ringMaterial->SetFloat3("u_Albedo", m_orbitColor);
    m_ringMaterial->SetFloat("u_Metallic", 0.0f);
    m_ringMaterial->SetFloat("u_Roughness", 0.6f);
    m_ringMaterial->SetFloat("u_AO", 1.0f);
    m_ringMaterial->SetFloat3("u_Emissive", m_orbitColor);
    m_ringMaterial->SetTexture("u_AlbedoMap", m_whiteTex);
    m_ringMaterial->SetTexture("u_MetallicMap", m_whiteTex);
    m_ringMaterial->SetTexture("u_RoughnessMap", m_whiteTex);
    m_ringMaterial->SetTexture("u_AOMap", m_whiteTex);
}

void SolarSystem::createPlanetMaterial(PlanetData& planet) {
    planet.Mat = Material::Create(m_pbrShader);
    planet.Mat->SetFloat3("u_Albedo", planet.Albedo);
    planet.Mat->SetFloat("u_Metallic", planet.Metallic);
    planet.Mat->SetFloat("u_Roughness", planet.Roughness);
    planet.Mat->SetFloat("u_AO", 1.0f);
    planet.Mat->SetFloat3("u_Emissive", planet.Emissive);
    planet.Mat->SetTexture("u_AlbedoMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_MetallicMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_RoughnessMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_AOMap", planet.AlbedoTexture);
}

void SolarSystem::OnDetach() {
    X_PROFILE_FUNCTION();

    m_pbrShader.reset();
    m_sphereMesh.reset();
    m_ringMesh.reset();
    m_ringMaterial.reset();
    m_whiteTex.reset();

    auto resetPlanet = [](PlanetData& p) {
        p.Mat.reset();
        p.AlbedoTexture.reset();
    };
    resetPlanet(m_sun);
    resetPlanet(m_moon);
    for (auto& planet : m_planets) {
        resetPlanet(planet);
    }
}

void SolarSystem::OnUpdate(Timestep ts) {
    X_PROFILE_FUNCTION();

    m_camera.OnUpdate(ts);

    float dt = ts.GetSeconds() * m_timeScale;

    m_sun.OrbitAngle += m_sun.RotationSpeed * dt;

    for (auto& planet : m_planets) {
        planet.OrbitAngle += planet.OrbitSpeed * dt;
    }

    m_moonOrbitAngle += m_moon.OrbitSpeed * dt;

    RenderCommand::SetClearColor({m_bgColor.r, m_bgColor.g, m_bgColor.b, 1.0f});
    RenderCommand::Clear();

    Renderer3D::ResetStats();

    Renderer3D::SetPointLight({0.0f, 0.0f, 0.0f}, {1.0f, 0.95f, 0.85f}, 60.0f, 2.5f);
    Renderer3D::SetLightCount(1);
    Renderer3D::SetExposure(1.0f);

    Renderer3D::BeginScene(m_camera);
    {
        // Sun
        {
            glm::mat4 sunTransform = glm::rotate(glm::mat4(1.0f), m_sun.OrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
                                     glm::scale(glm::mat4(1.0f), glm::vec3(m_sun.Size));
            Renderer3D::DrawMesh(m_sphereMesh, m_sun.Mat, sunTransform);
        }

        // Orbit rings
        for (auto& planet : m_planets) {
            glm::mat4 ringTransform = glm::scale(glm::mat4(1.0f), glm::vec3(planet.OrbitRadius));
            Renderer3D::DrawMesh(m_ringMesh, m_ringMaterial, ringTransform);
        }

        // Planets
        for (auto& planet : m_planets) {
            glm::vec3 orbitPos = {std::cos(planet.OrbitAngle) * planet.OrbitRadius, 0.0f,
                                  std::sin(planet.OrbitAngle) * planet.OrbitRadius};

            glm::mat4 planetTransform = glm::translate(glm::mat4(1.0f), orbitPos) *
                                        glm::rotate(glm::mat4(1.0f), planet.OrbitAngle * planet.RotationSpeed,
                                                    glm::vec3(0.0f, 1.0f, 0.0f)) *
                                        glm::scale(glm::mat4(1.0f), glm::vec3(planet.Size));

            Renderer3D::DrawMesh(m_sphereMesh, planet.Mat, planetTransform);

            if (&planet == &m_planets[2]) {
                glm::mat4 moonRingTransform = glm::translate(glm::mat4(1.0f), orbitPos) *
                                              glm::scale(glm::mat4(1.0f), glm::vec3(m_moon.OrbitRadius));
                Renderer3D::DrawMesh(m_ringMesh, m_ringMaterial, moonRingTransform);

                glm::vec3 moonOrbitPos = {std::cos(m_moonOrbitAngle) * m_moon.OrbitRadius, 0.0f,
                                          std::sin(m_moonOrbitAngle) * m_moon.OrbitRadius};

                glm::mat4 moonTransform = glm::translate(glm::mat4(1.0f), orbitPos + moonOrbitPos) *
                                          glm::rotate(glm::mat4(1.0f), m_moonOrbitAngle * m_moon.RotationSpeed,
                                                      glm::vec3(0.0f, 1.0f, 0.0f)) *
                                          glm::scale(glm::mat4(1.0f), glm::vec3(m_moon.Size));

                Renderer3D::DrawMesh(m_sphereMesh, m_moon.Mat, moonTransform);
            }
        }
    }
    Renderer3D::EndScene();
}

void SolarSystem::OnImguiRender() {
    X_PROFILE_FUNCTION();

    ImGui::Begin("Solar System");

    auto stats = Renderer3D::GetStats();
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Meshes: %d", stats.MeshCount);
    ImGui::SliderFloat("Time Scale", &m_timeScale, 0.1f, 5.0f, "%.1f");
    ImGui::Separator();
    ImGui::Text("Camera Distance: %.1f", m_camera.get_distance());
    if (ImGui::ColorEdit3("Orbit Color", glm::value_ptr(m_orbitColor))) {
        m_ringMaterial->SetFloat3("u_Albedo", m_orbitColor);
        m_ringMaterial->SetFloat3("u_Emissive", m_orbitColor);
    }
    if (ImGui::SliderFloat("Orbit Thickness", &m_orbitThickness, 0.002f, 0.05f, "%.3f")) {
        m_ringMesh = GeometryGenerator::CreateRing(1.0f - m_orbitThickness, 1.0f + m_orbitThickness, 128);
    }
    ImGui::ColorEdit3("Background", glm::value_ptr(m_bgColor));

    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 gizmoCenter(io.DisplaySize.x - 70.0f, 90.0f);
    float gizmoLen = 50.0f;

    glm::quat invOrientation = glm::inverse(m_camera.GetOrientation());
    auto projectAxis = [&](const glm::vec3& worldDir) -> ImVec2 {
        glm::vec3 viewDir = glm::rotate(invOrientation, worldDir);
        return {gizmoCenter.x + viewDir.x * gizmoLen, gizmoCenter.y - viewDir.y * gizmoLen};
    };

    ImVec2 xEnd = projectAxis({1.0f, 0.0f, 0.0f});
    ImVec2 yEnd = projectAxis({0.0f, 1.0f, 0.0f});
    ImVec2 zEnd = projectAxis({0.0f, 0.0f, 1.0f});

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddLine(gizmoCenter, xEnd, IM_COL32(255, 60, 60, 220), 3.0f);
    dl->AddLine(gizmoCenter, yEnd, IM_COL32(60, 255, 60, 220), 3.0f);
    dl->AddLine(gizmoCenter, zEnd, IM_COL32(60, 100, 255, 220), 3.0f);

    dl->AddText(ImVec2(xEnd.x + 4, xEnd.y - 8), IM_COL32(255, 60, 60, 255), "X");
    dl->AddText(ImVec2(yEnd.x + 4, yEnd.y - 8), IM_COL32(60, 255, 60, 255), "Y");
    dl->AddText(ImVec2(zEnd.x + 4, zEnd.y - 8), IM_COL32(60, 100, 255, 255), "Z");

    // Draw a ring at center for axes pointing toward/away from camera
    dl->AddCircle(gizmoCenter, 6.0f, IM_COL32(180, 180, 180, 180), 0, 2.0f);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mouse = ImGui::GetMousePos();
        float threshold = 14.0f;

        auto distToSeg = [](ImVec2 p, ImVec2 a, ImVec2 b) -> float {
            ImVec2 ab(b.x - a.x, b.y - a.y);
            ImVec2 ap(p.x - a.x, p.y - a.y);
            float len2 = ab.x * ab.x + ab.y * ab.y;
            if (len2 < 0.1f) {
                float dx = p.x - a.x;
                float dy = p.y - a.y;
                return std::sqrt(dx * dx + dy * dy);
            }
            float t = (ap.x * ab.x + ap.y * ab.y) / len2;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            float dx = p.x - (a.x + t * ab.x);
            float dy = p.y - (a.y + t * ab.y);
            return std::sqrt(dx * dx + dy * dy);
        };

        float dx = mouse.x - gizmoCenter.x;
        float dy = mouse.y - gizmoCenter.y;
        if (std::sqrt(dx * dx + dy * dy) < gizmoLen + 30.0f) {
            if (distToSeg(mouse, gizmoCenter, xEnd) < threshold) {
                m_camera.SetLookDirection({-1.0f, 0.0f, 0.0f});
            } else if (distToSeg(mouse, gizmoCenter, yEnd) < threshold) {
                m_camera.SetLookDirection({0.0f, -1.0f, 0.0f});
            } else if (distToSeg(mouse, gizmoCenter, zEnd) < threshold) {
                m_camera.SetLookDirection({0.0f, 0.0f, -1.0f});
            }
        }
    }
}

void SolarSystem::OnEvent(Event& e) {
    m_camera.OnEvent(e);
}
