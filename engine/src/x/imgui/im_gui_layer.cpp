//
// Created by rui ding on 2026/2/26.
//

#include "x/imgui/im_gui_layer.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "x/core/base.h"
#include "x/window.h"
#include "x/core/x_application.h"
#include "x/events/event.h"

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

void ImGuiLayer::OnAttach()
{
    X_PROFILE_FUNCTION();
    // 整合ImGUI
    IMGUI_CHECKVERSION();
    // 创建上下文
    ImGui::CreateContext();
    // 要启用的功能
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 不仅支持鼠标操作 还支持键盘操作 键盘导航
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  // 支持更改鼠标形状
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;   // 可以主动设置鼠标位置
    // 字体
    io.Fonts->AddFontFromFileTTF("asset/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
    io.FontDefault = io.Fonts->AddFontFromFileTTF("asset/fonts/opensans/OpenSans-Regular.ttf", 18.0f);
    // 主题
    ImGui::StyleColorsDark();
    XApplication& app = XApplication::Get();
    // ImGui绑定glfw 事件自动托管 不用自己手动管理
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(app.get_window().get_nativeWindow()), true);
    // ImGui绑定OpenGL
    ImGui_ImplOpenGL3_Init(X_GL_VERSION_CORE);
}

void ImGuiLayer::OnDetach()
{
    X_PROFILE_FUNCTION();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::OnUpdate(Timestep ts) {}

void ImGuiLayer::OnImguiRender() {}

void ImGuiLayer::OnEvent(Event& e)
{
    if (m_blockEvents)
    {
        ImGuiIO& io = ImGui::GetIO();
        e.Handled |= e.IsInCategory(kEventCategoryMouse) & io.WantCaptureMouse;
        e.Handled |= e.IsInCategory(kEventCategoryKeyboard) & io.WantCaptureKeyboard;
    }
}

void ImGuiLayer::Begin()
{
    X_PROFILE_FUNCTION();
    // 开始新帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End()
{
    X_PROFILE_FUNCTION();
    ImGuiIO&      io  = ImGui::GetIO();
    XApplication& app = XApplication::Get();
    io.DisplaySize =
        ImVec2(static_cast<float>(app.get_window().GetWidth()), static_cast<float>(app.get_window().GetHeight()));
    // 渲染
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
