//
// Created by rui ding on 2026/6/7.
//

#include "solar_system.h"

#include <glm/gtc/type_ptr.hpp>

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
             .Roughness = 0.3f};

    m_planets = {{
        {.Name = "mercury",
         .Albedo = {0.7f, 0.7f, 0.7f},
         .Size = 0.12f,
         .OrbitRadius = 5.0f,
         .OrbitSpeed = 4.0f,
         .RotationSpeed = 20.0f,
         .Metallic = 0.1f,
         .Roughness = 0.8f},
        {.Name = "venus",
         .Albedo = {0.9f, 0.75f, 0.35f},
         .Size = 0.25f,
         .OrbitRadius = 7.5f,
         .OrbitSpeed = 3.0f,
         .RotationSpeed = 15.0f,
         .Metallic = 0.0f,
         .Roughness = 0.7f},
        {.Name = "earth",
         .Albedo = {0.2f, 0.4f, 0.8f},
         .Size = 0.27f,
         .OrbitRadius = 10.0f,
         .OrbitSpeed = 2.0f,
         .RotationSpeed = 25.0f,
         .Metallic = 0.0f,
         .Roughness = 0.5f},
        {.Name = "mars",
         .Albedo = {0.8f, 0.3f, 0.2f},
         .Size = 0.15f,
         .OrbitRadius = 13.0f,
         .OrbitSpeed = 1.6f,
         .RotationSpeed = 22.0f,
         .Metallic = 0.0f,
         .Roughness = 0.9f},
        {.Name = "jupiter",
         .Albedo = {0.7f, 0.5f, 0.3f},
         .Size = 1.0f,
         .OrbitRadius = 17.0f,
         .OrbitSpeed = 0.8f,
         .RotationSpeed = 30.0f,
         .Metallic = 0.0f,
         .Roughness = 0.6f},
        {.Name = "saturn",
         .Albedo = {0.85f, 0.75f, 0.5f},
         .Size = 0.85f,
         .OrbitRadius = 21.0f,
         .OrbitSpeed = 0.6f,
         .RotationSpeed = 28.0f,
         .Metallic = 0.0f,
         .Roughness = 0.5f},
        {.Name = "uranus",
         .Albedo = {0.3f, 0.7f, 0.8f},
         .Size = 0.5f,
         .OrbitRadius = 25.0f,
         .OrbitSpeed = 0.4f,
         .RotationSpeed = 20.0f,
         .Metallic = 0.0f,
         .Roughness = 0.4f},
        {.Name = "neptune",
         .Albedo = {0.2f, 0.3f, 0.9f},
         .Size = 0.48f,
         .OrbitRadius = 29.0f,
         .OrbitSpeed = 0.3f,
         .RotationSpeed = 18.0f,
         .Metallic = 0.0f,
         .Roughness = 0.4f},
    }};

    m_moon = {.Name = "moon",
              .Albedo = {0.6f, 0.6f, 0.6f},
              .Size = 0.07f,
              .OrbitRadius = 0.6f,
              .OrbitSpeed = 10.0f,
              .RotationSpeed = 5.0f,
              .Metallic = 0.0f,
              .Roughness = 0.9f};
}

void SolarSystem::OnAttach() {
    X_PROFILE_FUNCTION();

    m_sphereMesh = GeometryGenerator::CreateSphere(0.5f, 48, 24);
    m_pbrShader = Shader::Create("asset/shader/Renderer3D_PBR.glsl");

    auto loadTex = [](const std::string& name) -> X::Ref<Texture2D> {
        std::string path = "asset/texture/" + name + ".jpg";
        X::Ref<Texture2D> tex = Texture2D::Create(path);
        if (!tex->IsLoaded()) {
            X_CORE_WARN("Failed to load texture: {}", path);
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
}

void SolarSystem::createPlanetMaterial(PlanetData& planet) {
    planet.Mat = Material::Create(m_pbrShader);
    planet.Mat->SetFloat3("u_Albedo", planet.Albedo);
    planet.Mat->SetFloat("u_Metallic", planet.Metallic);
    planet.Mat->SetFloat("u_Roughness", planet.Roughness);
    planet.Mat->SetFloat("u_AO", 1.0f);
    planet.Mat->SetTexture("u_AlbedoMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_MetallicMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_RoughnessMap", planet.AlbedoTexture);
    planet.Mat->SetTexture("u_AOMap", planet.AlbedoTexture);
}

void SolarSystem::OnDetach() {
    X_PROFILE_FUNCTION();

    m_pbrShader.reset();
    m_sphereMesh.reset();

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

    RenderCommand::SetClearColor({0.02f, 0.02f, 0.05f, 1.0f});
    RenderCommand::Clear();

    Renderer3D::ResetStats();

    Renderer3D::SetLightDirection({-0.2f, -0.8f, -0.3f});
    Renderer3D::SetLightColor({1.0f, 0.95f, 0.85f});
    Renderer3D::SetExposure(1.0f);

    Renderer3D::BeginScene(m_camera);
    {
        // Sun
        {
            glm::mat4 sunTransform = glm::rotate(glm::mat4(1.0f), m_sun.OrbitAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
                                     glm::scale(glm::mat4(1.0f), glm::vec3(m_sun.Size));
            Renderer3D::DrawMesh(m_sphereMesh, m_sun.Mat, sunTransform);
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

    ImGui::End();
}

void SolarSystem::OnEvent(Event& e) {
    m_camera.OnEvent(e);
}
