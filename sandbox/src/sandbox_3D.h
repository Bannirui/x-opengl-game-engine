//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include <engine.h>

#include <x/renderer/editor_camera.h>
#include <x/renderer/mesh.h>
#include <x/renderer/material.h>
#include <x/renderer/renderer_3D.h>

class Sandbox3D : public Layer {
public:
    Sandbox3D();
    ~Sandbox3D() override;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event& e) override;

private:
    EditorCamera m_editorCamera;
    X::Ref<Shader> m_pbrShader;
    X::Ref<Material> m_cubeMaterial;
    X::Ref<Mesh> m_cubeMesh;
    X::Ref<Mesh> m_planeMesh;
    X::Ref<Material> m_planeMaterial;
    X::Ref<TextureCube> m_envMap;

    glm::vec3 m_lightDirection{-0.5f, -1.0f, -0.3f};
    glm::vec3 m_lightColor{1.0f, 0.95f, 0.9f};
    float m_exposure{1.2f};
    float m_rotation{0.0f};
};
