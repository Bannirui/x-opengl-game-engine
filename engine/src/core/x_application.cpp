//
// Created by dingrui on 2/23/26.
//

#include "x/core/x_application.h"

#include "pch.h"
#include "x/core/layer.h"
#include "x/core/timestep.h"
#include "x/events/application_event.h"
#include "x/imgui/im_gui_layer.h"
#include "x/renderer/renderer.h"
#include "x/util/platform_util.h"
#include "x/window.h"

XApplication* XApplication::s_instance = nullptr;

XApplication::XApplication(const ApplicationSpecification& specification) : m_specification(specification) {
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    if (!m_specification.WorkingDirectory.empty()) {
        std::filesystem::current_path(m_specification.WorkingDirectory);
    }
    // 窗体
    m_window = Window::Create(WindowProps(m_specification.Name));
    m_window->SetEventCallback([this](Event& e) {
        // 窗口事件丢给引擎 引擎不会立即处理 先缓存到队列里面 延迟批量一起处理
        this->OnEvent(e);
    });

    Renderer::Init();

    auto imGuiLayer = X::CreateScope<ImGuiLayer>();
    m_ImGuiLayer = imGuiLayer.get();
    PushOverlay(std::move(imGuiLayer));
}

XApplication::~XApplication() {
    X_PROFILE_FUNCTION();
    Renderer::Shutdown();
}

void XApplication::OnEvent(Event& e) {
    X_PROFILE_FUNCTION();
    m_eventQueue.push(e.Clone());
}

void XApplication::ProcessEvents() {
    // 把攒着的事件一次性处理掉
    while (!m_eventQueue.empty()) {
        auto& e = *m_eventQueue.front();

        switch (e.GetEventType()) {
            case EventType::kWindowClose:
                m_running = false;
                break;
            case EventType::kWindowResize: {
                WindowResizeEvent& resize = static_cast<WindowResizeEvent&>(e);
                if (resize.get_width() == 0 || resize.get_height() == 0) {
                    m_minimized = true;
                } else {
                    m_minimized = false;
                    Renderer::OnWindowResize(resize.get_width(), resize.get_height());
                }
                break;
            }
            default:
                break;
        }
        // 反向遍历 每层都看看当前事件自己感不感兴趣 反向的原因是保证ImGui层可以比渲染层优先吞掉键鼠事件
        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
            if (e.Handled) break;
            if (!(*it)->IsInterestedIn(e)) continue;
            (*it)->OnEvent(e);
        }

        m_eventQueue.pop();
    }
}

void XApplication::PushLayer(X::Scope<Layer> layer) {
    X_PROFILE_FUNCTION();
    layer->OnAttach();
    m_layerStack.PushLayer(std::move(layer));
}

void XApplication::PushOverlay(X::Scope<Layer> overlay) {
    X_PROFILE_FUNCTION();
    overlay->OnAttach();
    m_layerStack.PushOverlay(std::move(overlay));
}

void XApplication::Close() {
    m_running = false;
}

void XApplication::run() {
    X_PROFILE_FUNCTION();
    while (m_running) {
        X_PROFILE_SCOPE("RunLoop");
        // 每一帧都一次性处理一下攒着的事件
        ProcessEvents();

        float time = Time::GetTime();
        Timestep timestep = time - m_lastFrameTime;
        m_lastFrameTime = time;

        if (!m_minimized) {
            {
                X_PROFILE_SCOPE("LayerStack OnUpdate");
                for (auto& layer : m_layerStack) {
                    layer->OnUpdate(timestep);
                }
            }
            m_ImGuiLayer->Begin();
            {
                X_PROFILE_SCOPE("LayerStack OnImguiRender");
                for (auto& layer : m_layerStack) {
                    layer->OnImguiRender();
                }
            }
            m_ImGuiLayer->End();
        }
        m_window->OnUpdate();
    }
}
