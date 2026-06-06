//
// Created by rui ding on 2026/6/6.
//

#pragma once

#include <glm/glm.hpp>

#include <x/core/base.h>

#include <engine.h>

class VertexArray;

/**
 * 从最简单图形画起 逐步排查渲染管线问题
 *
 * 测试级别 (ImGui 面板切换):
 *   Level 0: 仅清屏 (验证 glClear 正常)
 *   Level 1: 三角形 (FlatColor shader + 手动 VAO/VBO, 绕开 Renderer3D)
 *   Level 2: 立方体 (FlatColor shader + 手动 VAO/VBO/IBO)
 *   Level 3: 立方体 (PBR shader + Renderer3D 完整管线)
 */
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
    void InitLevel1Triangle();
    void InitLevel2Cube();
    void InitLevel3PBR();

    void DrawLevel1Triangle();
    void DrawLevel2Cube();
    void DrawLevel3PBR(Timestep ts);

    int m_testLevel = 3;
    float m_rotation = 0.0f;

    // Level 1/2: 手动 VAO/VBO/IBO + FlatColor shader
    X::Ref<Shader> m_flatColorShader;
    X::Ref<VertexArray> m_triangleVAO;
    X::Ref<VertexArray> m_cubeVAO;
    uint32_t m_triangleIndexCount = 0;
    uint32_t m_cubeIndexCount = 0;

    // Level 3: PBR + Renderer3D (沿用之前 sandbox_3D 的成员)
    X::Ref<Shader> m_pbrShader;
    X::Ref<Mesh> m_cubeMesh;
    X::Ref<Material> m_cubeMaterial;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
};
