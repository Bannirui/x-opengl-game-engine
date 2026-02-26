//
// Created by rui ding on 2026/2/26.
//

#include "x/imgui/im_gui_layer.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include "x/core.h"
#include "x/window.h"
#include "x/x_application.h"
#include "x/events/event.h"

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::OnAttach()
{
    // 整合ImGUI
    ImGui::CreateContext();
    // 主题
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO();
    // 支持更改鼠标形状
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    // 可以主动设置鼠标位置
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    // 不仅支持鼠标操作 还支持键盘操作
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // ImGui绑定OpenGL
    ImGui_ImplOpenGL3_Init(X_GL_VERSION);
}

void ImGuiLayer::OnDetach() {}

void ImGuiLayer::OnUpdate()
{
    ImGuiIO      &io  = ImGui::GetIO();
    XApplication &app = XApplication::Get();
    io.DisplaySize =
        ImVec2(static_cast<float>(app.get_window().GetWidth()), static_cast<float>(app.get_window().GetHeight()));

    float time   = static_cast<float>(glfwGetTime());
    io.DeltaTime = m_time > 0.0f ? (time - m_time) : (1.0f / 60.0f);
    m_time       = time;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    static bool show{true};
    ImGui::ShowDemoWindow(&show);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::OnEvent(const Event &e) {}
