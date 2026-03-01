//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x_engine.h"

class Sandbox2D : public Layer
{
public:
    Sandbox2D();

    ~Sandbox2D() override;

    void OnAttach() override;

    void OnDetach() override;

    void OnUpdate(Timestep ts) override;

    void OnImguiRender() override;

    void OnEvent(Event &e) override;

private:
    OrthographicCameraController m_cameraController;
    X::Ref<VertexArray>          m_squareVA;
    X::Ref<Shader>               m_flatShader;
    X::Ref<Texture2D>            m_checkerboardTexture;
    glm::vec4                    m_squareColor = {0.2f, 0.3f, 0.8f, 1.0f};

    struct ProfileResult
    {
        const char *name;
        float       time;  // milliseconds
    };
    std::vector<ProfileResult> m_profileResults;
};
