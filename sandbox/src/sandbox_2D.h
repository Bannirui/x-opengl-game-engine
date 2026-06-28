//
// Created by dingrui on 2/28/26.
//

#pragma once

#include <engine.h>

class Sandbox2D : public Layer {
public:
    Sandbox2D();

    ~Sandbox2D() override;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event& e) override;

private:
    OrthographicCameraController m_cameraController;
    Ref<VertexArray> m_squareVA;
    Ref<Shader> m_flatShader;
    Ref<Texture2D> m_checkerboardTexture;
    Ref<Texture2D> m_cppTexture;
    glm::vec4 m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};

    std::vector<ProfileResult> m_profileResults;
};
