//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include "x/events/event.h"
#include "x/scene/scene_serializer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <x/core/mac_input.h>
#include <x/core/x_application.h>
#include <x/imgui/im_gui_layer.h>
#include <x/renderer/frame_buffer.h>
#include <x/renderer/render_command.h>
#include <x/renderer/renderer_2D.h>
#include <x/renderer/texture.h>
#include <x/scene/scene.h>
#include <x/util/platform_util.h>
#include <x/events/key_event.h>
#include <x/scene/scene_serializer.h>

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

#if 0
    // Entity
    auto square = m_activeScene->CreateEntity("Green Square");
    square.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
    auto redSquare = m_activeScene->CreateEntity("Red Square");
    redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});
    m_squareEntity = square;

    m_cameraEntity = m_activeScene->CreateEntity("Camera A");
    m_cameraEntity.AddComponent<CameraComponent>();

    m_secondCamera = m_activeScene->CreateEntity("Camera B");
    auto& cc       = m_secondCamera.AddComponent<CameraComponent>();
    cc.m_primary   = false;

    class CameraController : public ScriptableEntity
    {
    public:
        void OnCreate() override
        {
            auto& translation = GetComponent<TransformComponent>().m_translation;
            translation.x     = rand() % 10 - 5.0f;
        }
        void OnDestroy() override {}
        void OnUpdate(Timestep ts) override
        {
            auto& translation = GetComponent<TransformComponent>().m_translation;
            float speed{5.0f};
            if (Input::IsKeyPressed(X_KEY::A))
            {
                translation.x -= speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::D))
            {
                translation.x += speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::W))
            {
                translation.y += speed * ts;
            }
            if (Input::IsKeyPressed(X_KEY::S))
            {
                translation.y -= speed * ts;
            }
        }
    };
    m_cameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
    m_secondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif
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
    ImGuiStyle& style       = ImGui::GetStyle();
    float       minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x   = 370.0f;

    style.WindowMinSize.x = minWinSizeX;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
                newScene();
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O"))
            {
                openScene();
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
            {
                saveSceneAs();
            }
            if (ImGui::MenuItem("Exit"))
            {
                XApplication::Get().Close();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    m_sceneHierarchyPanel.OnImGuiRender();

    ImGui::Begin("Stats");

    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
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
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressEvent>(
        [this](KeyPressEvent& e)
        {
            return this->onKeyPressed(e);
        });
}

bool EditorLayer::onKeyPressed(KeyPressEvent& e)
{
    if (e.is_repeat())
    {
        return false;
    }
    bool control = Input::IsKeyPressed(X_KEY::LeftControl) || Input::IsKeyPressed(X_KEY::RightControl);
    bool shift   = Input::IsKeyPressed(X_KEY::LeftShift) || Input::IsKeyPressed(X_KEY::RightShift);
    switch (e.get_keyCode())
    {
        case X_KEY::N:
        {
            if (control)
            {
                // ctrl+N
                newScene();
            }
            break;
        }
        case X_KEY::O:
        {
            if (control)
            {
                // ctrl+N
                openScene();
            }
            break;
        }
        case X_KEY::S:
        {
            if (control && shift)
            {
                // ctrl+shift+S
                saveSceneAs();
            }
            break;
        }
    }
    return true;
}

void EditorLayer::newScene()
{
    m_activeScene = X::CreateRef<Scene>();
    m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
    m_sceneHierarchyPanel.set_context(m_activeScene);
}

void EditorLayer::openScene()
{
    std::string filepath = FileDialog::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
    if (!filepath.empty())
    {
        m_activeScene = X::CreateRef<Scene>();
        m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        m_sceneHierarchyPanel.set_context(m_activeScene);

        SceneSerializer serializer(m_activeScene);
        serializer.Deserialize(filepath);
    }
}

void EditorLayer::saveSceneAs()
{
    std::string filepath = FileDialog::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0");
    if (!filepath.empty())
    {
        SceneSerializer serializer(m_activeScene);
        serializer.Serialize(filepath);
    }
}
