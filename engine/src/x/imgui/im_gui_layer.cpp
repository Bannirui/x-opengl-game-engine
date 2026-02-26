//
// Created by rui ding on 2026/2/26.
//

#include "x/imgui/im_gui_layer.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "x/core.h"
#include "x/window.h"
#include "x/events/event.h"
#include "x/x_application.h"
#include "x/events/mouse_event.h"
#include "x/events/key_event.h"

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach()
{
    // 整合ImGUI
    IMGUI_CHECKVERSION();
    // 创建上下文
    ImGui::CreateContext();
    // 主题
    ImGui::StyleColorsDark();
    // 要启用的功能
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 不仅支持鼠标操作 还支持键盘操作 键盘导航
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  // 支持更改鼠标形状
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;   // 可以主动设置鼠标位置
    XApplication &app = XApplication::Get();
    // ImGui绑定glfw 事件自动托管 不用自己手动管理
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(app.get_window().get_nativeWindow()), true);
    // ImGui绑定OpenGL
    ImGui_ImplOpenGL3_Init(X_GL_VERSION);
}

void ImGuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnUpdate()
{
    ImGuiIO      &io  = ImGui::GetIO();
    XApplication &app = XApplication::Get();
    io.DisplaySize =
        ImVec2(static_cast<float>(app.get_window().GetWidth()), static_cast<float>(app.get_window().GetHeight()));

    float time   = static_cast<float>(glfwGetTime());
    io.DeltaTime = m_time > 0.0f ? (time - m_time) : (1.0f / 60.0f);
    m_time       = time;

    // 开始新帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // UI逻辑
    static bool show{true};
    ImGui::ShowDemoWindow(&show);

    // 渲染
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::OnEvent(Event &e)
{
}