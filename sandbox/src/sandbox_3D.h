//
// Created by rui ding on 2026/6/6.
//

#pragma once

#include <glm/glm.hpp>

#include <x/core/base.h>

#include <engine.h>

class VertexArray;

class Sandbox3D : public Layer {
public:
    Sandbox3D();
    ~Sandbox3D() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event& e) override;

private:
    void InitLevel3PBR();

    float m_rotation = 0.0f;

    X::Ref<Shader> m_pbrShader;
    X::Ref<Mesh> m_cubeMesh;
    X::Ref<Material> m_cubeMaterial;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
};
