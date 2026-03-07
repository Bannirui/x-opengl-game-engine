//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <x/core/input.h>
#include <x/core/x_application.h>
#include <x/events/event.h>
#include <x/events/key_event.h>
#include <x/events/mouse_event.h>
#include <x/imgui/im_gui_layer.h>
#include <x/math/math.h>
#include <x/renderer/frame_buffer.h>
#include <x/renderer/render_command.h>
#include <x/renderer/renderer_2D.h>
#include <x/renderer/texture.h>
#include <x/scene/component.h>
#include <x/scene/scene.h>
#include <x/scene/scene_serializer.h>
#include <x/util/platform_util.h>

EditorLayer::EditorLayer()
    : Layer("EditorLayer"), m_cameraController(1280.0f / 720.0f), m_squareColor({0.2f, 0.3f, 0.4f, 1.0f})
{
}

void EditorLayer::OnAttach()
{
    X_PROFILE_FUNCTION();

    m_checkerboardTexture = Texture2D::Create("asset/texture/Checkerboard.png");

    FramebufferSpecification fbSpec;
    fbSpec.m_width       = 1280;
    fbSpec.m_height      = 720;
    fbSpec.m_attachments = {FramebufferTextureFormat::kRGBA8, FramebufferTextureFormat::kRED_INTEGER,
                            FramebufferTextureFormat::kDepth};
    m_framebuffer        = FrameBuffer::Create(fbSpec);

    m_activeScene        = X::CreateRef<Scene>();
    auto commandLineArgs = XApplication::Get().get_commandLineArgs();
    if (commandLineArgs.Count > 1)
    {
        auto            sceneFilePath = commandLineArgs[1];
        SceneSerializer serializer(m_activeScene);
        serializer.Deserialize(sceneFilePath);
    }
    m_editorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

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
    if (FramebufferSpecification spec = m_framebuffer->GetSpecification();
        m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f &&  // zero sized framebuffer is invalid
        (spec.m_width != m_viewportSize.x || spec.m_height != m_viewportSize.y))
    {
        m_framebuffer->Resize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        m_cameraController.OnResize(m_viewportSize.x, m_viewportSize.y);
        m_editorCamera.SetViewportSize(m_viewportSize.x, m_viewportSize.y);
        m_activeScene->OnViewportResize(static_cast<uint32_t>(m_viewportSize.x),
                                        static_cast<uint32_t>(m_viewportSize.y));
    }
    // Update
    if (m_viewportFocused)
    {
        m_cameraController.OnUpdate(ts);
    }
    m_editorCamera.OnUpdate(ts);
    // Render
    Renderer2D::ResetStats();
    m_framebuffer->Bind();
    RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    RenderCommand::Clear();
    // Clear entity ID attachment to -1
    m_framebuffer->ClearAttachment(1, -1);

    // Update scene
    m_activeScene->OnUpdateEditor(ts, m_editorCamera);

    auto [mx, my] = ImGui::GetMousePos();
    mx -= m_viewportBounds[0].x;
    my -= m_viewportBounds[0].y;
    glm::vec2 viewportSize = m_viewportBounds[1] - m_viewportBounds[0];
    my                     = viewportSize.y - my;
    int mouseX             = static_cast<int>(mx);
    int mouseY             = static_cast<int>(my);
    if (mouseX >= 0 && my >= 0 && mouseX < static_cast<int>(viewportSize.x) &&
        mouseY < static_cast<int>(viewportSize.y))
    {
        int pixelData   = m_framebuffer->ReadPixel(1, mouseX, mouseY);
        m_hoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_activeScene.get());
    }

    m_framebuffer->Unbind();
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
    std::string name = "None";
    if (m_hoveredEntity)
    {
        name = m_hoveredEntity.GetComponent<TagComponent>().m_tag;
    }
    ImGui::Text("Hovered Entity: %s", name.c_str());

    auto stats = Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    ImGui::Begin("Viewport");
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset    = ImGui::GetWindowPos();
    m_viewportBounds[0]    = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
    m_viewportBounds[1]    = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();
    XApplication::Get().get_ImGuiLayer()->BlockEvents(!m_viewportFocused && !m_viewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_viewportSize           = {viewportPanelSize.x, viewportPanelSize.y};

    uint64_t textureID = m_framebuffer->GetColorAttachmentRendererID();
    ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{m_viewportSize.x, m_viewportSize.y}, ImVec2{0, 1},
                 ImVec2{1, 0});

    // Gizmos
    Entity selectedEntity = m_sceneHierarchyPanel.get_selectedEntity();
    if (selectedEntity && m_gizmoType != -1)
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_viewportBounds[0].x, m_viewportBounds[0].y, m_viewportBounds[1].x - m_viewportBounds[0].x,
                          m_viewportBounds[1].y - m_viewportBounds[0].y);

        // Camera
        // auto             cameraEntity     = m_activeScene->GetPrimaryCameraEntity();
        // const auto&      camera           = cameraEntity.GetComponent<CameraComponent>().m_camera;
        // const glm::mat4& cameraProjection = camera.get_projection();
        // glm::mat4        cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
        // Editor camera
        const glm::mat4 cameraProjection = m_editorCamera.get_projection();
        glm::mat4       cameraView       = m_editorCamera.get_viewMatrix();

        // Entity transform
        auto&     tc        = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        // Snapping
        bool  snap      = Input::IsKeyPressed(X::KEY::LeftControl);
        float snapValue = 0.5f;  // Snap to 0.5m for translation/scale
        // Snap to 45 degrees for rotation
        if (m_gizmoType == ImGuizmo::OPERATION::ROTATE) snapValue = 45.0f;

        float snapValues[3] = {snapValue, snapValue, snapValue};

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                             (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr,
                             snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            MATH::DecomposeTransform(transform, translation, rotation, scale);

            glm::vec3 deltaRotation = rotation - tc.m_rotation;
            tc.m_translation        = translation;
            tc.m_rotation += deltaRotation;
            tc.m_scale = scale;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::End();
}

void EditorLayer::OnEvent(Event& e)
{
    m_cameraController.OnEvent(e);
    m_editorCamera.OnEvent(e);
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressEvent>(
        [this](KeyPressEvent& e)
        {
            return this->onKeyPressed(e);
        });
    dispatcher.Dispatch<MouseButtonPressedEvent>(
        [this](MouseButtonPressedEvent& e)
        {
            return this->onMouseButtonPressed(e);
        });
}

bool EditorLayer::onKeyPressed(KeyPressEvent& e)
{
    if (e.is_repeat())
    {
        return false;
    }
    bool control = Input::IsKeyPressed(X::KEY::LeftControl) || Input::IsKeyPressed(X::KEY::RightControl);
    bool shift   = Input::IsKeyPressed(X::KEY::LeftShift) || Input::IsKeyPressed(X::KEY::RightShift);
    switch (e.get_keyCode())
    {
        case X::KEY::N:
        {
            if (control)
            {
                // ctrl+N
                newScene();
            }
            break;
        }
        case X::KEY::O:
        {
            if (control)
            {
                // ctrl+N
                openScene();
            }
            break;
        }
        case X::KEY::S:
        {
            if (control && shift)
            {
                // ctrl+shift+S
                saveSceneAs();
            }
            break;
        }
        // Gizmos
        case X::KEY::Q:
        {
            if (!ImGuizmo::IsUsing())
            {
                m_gizmoType = -1;
            }
            break;
        }
        case X::KEY::W:
        {
            if (!ImGuizmo::IsUsing())
            {
                m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
            }
            break;
        }
        case X::KEY::E:
        {
            if (!ImGuizmo::IsUsing())
            {
                m_gizmoType = ImGuizmo::OPERATION::ROTATE;
            }
            break;
        }
        case X::KEY::R:
        {
            if (!ImGuizmo::IsUsing())
            {
                m_gizmoType = ImGuizmo::OPERATION::SCALE;
            }
            break;
        }
    }
    return true;
}

bool EditorLayer::onMouseButtonPressed(MouseButtonPressedEvent& e)
{
    if (e.get_mouseButton() == X::MOUSE::ButtonLeft)
    {
        if (m_viewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(X::KEY::LeftAlt))
        {
            m_sceneHierarchyPanel.set_selectedEntity(m_hoveredEntity);
        }
    }
    return false;
}

void EditorLayer::newScene()
{
    m_activeScene = X::CreateRef<Scene>();
    m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
    m_sceneHierarchyPanel.set_context(m_activeScene);
}

void EditorLayer::openScene()
{
    std::optional<std::string> filepath = FileDialog::OpenFile("X Scene (*.x)\0*.x\0");
    if (filepath)
    {
        m_activeScene = X::CreateRef<Scene>();
        m_activeScene->OnViewportResize((uint32_t)m_viewportSize.x, (uint32_t)m_viewportSize.y);
        m_sceneHierarchyPanel.set_context(m_activeScene);

        SceneSerializer serializer(m_activeScene);
        serializer.Deserialize(*filepath);
    }
}

void EditorLayer::saveSceneAs()
{
    std::optional<std::string> filepath = FileDialog::SaveFile("X Scene (*.x)\0*.x\0");
    if (filepath)
    {
        SceneSerializer serializer(m_activeScene);
        serializer.Serialize(*filepath);
    }
}
