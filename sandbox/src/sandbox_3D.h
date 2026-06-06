//
// Created by rui ding on 2026/6/6.
//

#pragma once

#include <x/renderer/camera/editor_camera.h>
#include <x/renderer/material.h>
#include <x/renderer/mesh.h>

#include <engine.h>

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

    EditorCamera m_camera;
    float m_rotation = 0.0f;

    // 渲染立方体要用到
    X::Ref<Shader> m_cubeShader;
    X::Ref<Mesh> m_cubeMesh;
    X::Ref<Material> m_cubeMaterial;
};
