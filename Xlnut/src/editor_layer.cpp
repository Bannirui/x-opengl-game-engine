//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "x/core/input.h"
#include "x/core/x_application.h"
#include "x/imgui/im_gui_layer.h"
#include "x/renderer/frame_buffer.h"
#include "x/renderer/render_command.h"
#include "x/renderer/renderer_2D.h"
#include "x/renderer/texture.h"
#include "x/scene/component.h"
#include "x/scene/scene.h"
#include "x/scene/entity.h"

EditorLayer::EditorLayer()
    : Layer("EditorLayer"), m_cameraController(1280.0f / 720.0f), m_squareColor({0.2f, 0.3f, 0.4f, 1.0f})
{
}

void EditorLayer::OnAttach()
{
    X_PROFILE_FUNCTION();

    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");

    FramebufferSpecification fbSpec;
    fbSpec.width  = 1280;
    fbSpec.height = 720;
    m_frameBuffer = FrameBuffer::Create(fbSpec);

    m_activeScene = X::CreateRef<Scene>();

    // Entity
    auto square = m_activeScene->CreateEntity("Green Square");
    square.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
    auto redSquare = m_activeScene->CreateEntity("Red Square");
    redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
    m_squareEntity = square;

    m_cameraEntity = m_activeScene->CreateEntity("Camera Entity");
    m_cameraEntity.AddComponent<CameraComponent>();

    m_secondCamera = m_activeScene->CreateEntity("Clip-Space Entity");
    auto& cc       = m_secondCamera.AddComponent<CameraComponent>();
    cc.m_primary   = false;

    class CameraController : public ScriptableEntity
    {
    public:
        void OnCreate() override
        {
            auto& transform = GetComponent<TransformComponent>().m_transform;
            transform[3][0] = rand() % 10 - 5.0f;
        }
        void OnDestroy() override {}
        void OnUpdate(Timestep ts) override
        {
            auto& transform = GetComponent<TransformComponent>().m_transform;
            float speed{5.0f};
            if (Input::IsKeyPressed(X_KEY::A))
            {
                transform[3][0] -= speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::D))
            {
                transform[3][0] += speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::W))
            {
                transform[3][1] += speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::S))
            {
                transform[3][1] -= speed * ts;
            }
        }
    };
    m_cameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
    m_secondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
    m_sceneHierarchyPanel.set_context(m_activeScene);
}

void EditorLayer::OnDetach()
{
    X_PROFILE_FUNCTION();
}

void EditorLayer::OnUpdate(Timestep ts)
{
    X_PROFILE_FUNCTION();
    // Resize
    if (FramebufferSpecification spec = m_frameBuffer->GetSpecification();
        m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f &&  // zero sized framebuffer is invalid
        (spec.width != m_viewportSize.x || spec.height != m_viewportSize.y))
    {
        m_frameBuffer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        m_cameraController.OnResize(m_viewportSize.x, m_viewportSize.y);
        m_activeScene->OnViewportResize(static_cast<uint32_t>(m_viewportSize.x),
                                        static_cast<uint32_t>(m_viewportSize.y));
    }
    // Update
    if (m_viewportFocused)
    {
        m_cameraController.OnUpdate(ts);
    }
    // Render
    Renderer2D::ResetStats();
    m_frameBuffer->Bind();
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    RenderCommand::Clear();

    // Update scene
    m_activeScene->OnUpdate(ts);

    m_frameBuffer->Unbind();
}

void EditorLayer::OnImguiRender()
{
    X_PROFILE_FUNCTION();

    // Note: Switch this to true to enable dockspace
    static bool dockspaceOpen             = true;
    static bool opt_fullscreen_persistant = true;
    bool        opt_fullscreen            = opt_fullscreen_persistant;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen) ImGui::PopStyleVar(2);

    // DockSpace
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            // ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

            if (ImGui::MenuItem("Exit"))
            {
                XApplication::Get().Close();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    m_sceneHierarchyPanel.OnImGuiRender();

    ImGui::Begin("Settings");

    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

    if (m_squareEntity)
    {
        ImGui::Separator();
        auto& tag = m_squareEntity.GetComponent<TagComponent>().m_tag;
        ImGui::Text("%s", tag.c_str());

        auto& squareColor = m_squareEntity.GetComponent<SpriteRendererComponent>().m_color;
        ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
        ImGui::Separator();
    }

    ImGui::DragFloat3("Camera Transform",
                      glm::value_ptr(m_cameraEntity.GetComponent<TransformComponent>().m_transform[3]));
    if (ImGui::Checkbox("CameraA", &m_primaryCamera))
    {
        m_cameraEntity.GetComponent<CameraComponent>().m_primary = m_primaryCamera;
        m_secondCamera.GetComponent<CameraComponent>().m_primary = !m_primaryCamera;
    }
    {
        auto& camera    = m_secondCamera.GetComponent<CameraComponent>().m_camera;
        float orthoSize = camera.get_orthographicSize();
        if (ImGui::DragFloat("Second Camera Ortho Size", &orthoSize))
        {
            camera.SetOrthographicSize(orthoSize);
        }
    }
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");

    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();
    XApplication::Get().get_ImGuiLayer()->BlockEvents(!m_viewportFocused || !m_viewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_viewportSize           = {viewportPanelSize.x, viewportPanelSize.y};

    uint64_t textureID = m_frameBuffer->GetColorAttachmentRendererID();
    ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{m_viewportSize.x, m_viewportSize.y}, ImVec2{0, 1},
                 ImVec2{1, 0});
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::End();
}

void EditorLayer::OnEvent(Event& e)
{
    m_cameraController.OnEvent(e);
}
