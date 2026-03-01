//
// Created by dingrui on 2/28/26.
//

#include "sandbox_2D.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.inl>

#include <chrono>

template <typename Fn>
class Timer
{
public:
    Timer(const char *name, Fn &&func) : m_name(name), m_func(std::forward<Fn>(func))
    {
        m_startTimepoint = std::chrono::steady_clock::now();
    }

    ~Timer()
    {
        if (!m_stopped)
        {
            stop();
        }
    }

    Timer(const Timer &)            = delete;
    Timer &operator=(const Timer &) = delete;

private:
    void stop()
    {
        auto end       = std::chrono::steady_clock::now();
        m_stopped      = true;
        float duration = std::chrono::duration<float, std::milli>(end - m_startTimepoint).count();
        m_func({m_name, duration});
    }

private:
    const char                                        *m_name;
    Fn                                                 m_func;
    std::chrono::time_point<std::chrono::steady_clock> m_startTimepoint;
    bool                                               m_stopped{false};
};

#define PROFILE_SCOPE(name) \
    Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_profileResults.push_back(profileResult); })

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) {}

Sandbox2D::~Sandbox2D() {}

void Sandbox2D::OnAttach()
{
    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(Timestep ts)
{
    PROFILE_SCOPE("Sandbox2D::OnUpdate");

    // update
    {
        PROFILE_SCOPE("CameraController::OnUpdate");
        m_cameraController.OnUpdate(ts);
    }

    // render
    {
        PROFILE_SCOPE("Renderer Prep");
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
    }

    {
        PROFILE_SCOPE("Renderer Draw");
        Renderer2D::BeginScene(m_cameraController.get_camera());
        Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {0.8f, 0.2f, 0.3f, 1.0f});
        Renderer2D::DrawQuad({0.5, -0.5f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
        Renderer2D::DrawQuad({0.0f, 0.0f, -1.0f}, {10.0f, 10.0f}, m_checkerboardTexture);
        Renderer2D::EndScene();
    }
}

void Sandbox2D::OnImguiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Square color", glm::value_ptr(m_squareColor));
    {
        for (auto &result : m_profileResults)
        {
            char label[50];
            strcpy(label, "%.3fms ");
            strcat(label, result.name);
            ImGui::Text(label, result.time);
        }
    }
    ImGui::End();
}

void Sandbox2D::OnEvent(Event &e)
{
    m_cameraController.OnEvent(e);
}
