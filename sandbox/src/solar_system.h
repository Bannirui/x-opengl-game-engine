//
// Created by rui ding on 2026/6/7.
//

#pragma once

#include <x/renderer/camera/editor_camera.h>
#include <x/renderer/material.h>
#include <x/renderer/mesh.h>

#include <engine.h>

#include <array>
#include <string>

struct PlanetData {
    std::string Name;
    glm::vec3 Albedo;
    float Size;
    float OrbitRadius;
    float OrbitSpeed;
    float RotationSpeed;
    float Metallic;
    float Roughness;
    glm::vec3 Emissive;
    X::Ref<Material> Mat;
    X::Ref<Texture2D> AlbedoTexture;
    float OrbitAngle = 0.0f;
};

class SolarSystem : public Layer {
public:
    SolarSystem();
    ~SolarSystem() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event& e) override;

private:
    void createPlanetMaterial(PlanetData& planet);

    EditorCamera m_camera;

    X::Ref<Shader> m_pbrShader;
    X::Ref<Mesh> m_sphereMesh;
    X::Ref<Mesh> m_ringMesh;
    X::Ref<Material> m_ringMaterial;
    X::Ref<Texture2D> m_whiteTex;

    PlanetData m_sun;
    std::array<PlanetData, 8> m_planets;

    PlanetData m_moon;
    float m_moonOrbitAngle = 0.0f;

    float m_timeScale = 1.0f;
    glm::vec3 m_orbitColor = {0.15f, 0.2f, 0.3f};
    float m_orbitThickness = 0.006f;
    glm::vec3 m_bgColor = {0.02f, 0.02f, 0.05f};
};
