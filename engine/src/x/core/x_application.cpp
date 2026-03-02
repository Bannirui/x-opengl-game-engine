//
// Created by dingrui on 2/23/26.
//

#include "x/core/x_application.h"

#include "pch.h"

#include "x/renderer/renderer.h"
#include "x/core/layer.h"
#include "x/events/application_event.h"
#include "x/window.h"
#include "x/imgui/im_gui_layer.h"
#include "x/core/timestep.h"

XApplication *XApplication::s_instance = nullptr;

XApplication::XApplication(const std::string &name)
{
    X_PROFILE_FUNCTION();
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    m_window   = Window::Create(WindowProps(name));
    m_window->SetEventCallback([this](Event &e) { this->OnEvent(e); });

    Renderer::Init();

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);
}

XApplication::~XApplication()
{
    X_PROFILE_FUNCTION();
    Renderer::Shutdown();
}

void XApplication::OnEvent(Event &e)
{
    X_PROFILE_FUNCTION();
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &e) { return this->onWindowClose(e); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) { return this->onWindowResize(e); });
    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it)
    {
        if (e.Handled)
        {
            break;
        }
        (*it)->OnEvent(e);
    }
}

void XApplication::PushLayer(Layer *layer)
{
    X_PROFILE_FUNCTION();
    m_layerStack.PushLayer(layer);
    layer->OnAttach();
}

void XApplication::PushOverlay(Layer *layer)
{
    X_PROFILE_FUNCTION();
    m_layerStack.PushOverlay(layer);
    layer->OnAttach();
}

void XApplication::Close()
{
    m_running = false;
}

void XApplication::run()
{
    X_PROFILE_FUNCTION();
    while (m_running)
    {
        X_PROFILE_SCOPE("RunLoop");
        float    time     = static_cast<float>(glfwGetTime());
        Timestep timestep = time - m_lastFrameTime;
        m_lastFrameTime   = time;

        if (!m_minimized)
        {
            {
                X_PROFILE_SCOPE("LayerStack OnUpdate");
                for (Layer *layer : m_layerStack)
                {
                    layer->OnUpdate(timestep);
                }
            }
            m_ImGuiLayer->Begin();
            {
                X_PROFILE_SCOPE("LayerStack OnImguiRender");
                for (Layer *layer : m_layerStack)
                {
                    layer->OnImguiRender();
                }
            }
            m_ImGuiLayer->End();
        }
        m_window->OnUpdate();
    }
}

bool XApplication::onWindowClose(WindowCloseEvent &e)
{
    m_running = false;
    return true;
}

bool XApplication::onWindowResize(WindowResizeEvent &e)
{
    X_PROFILE_FUNCTION();
    if (e.get_width() == 0 || e.get_height() == 0)
    {
        m_minimized = true;
        return false;
    }
    m_minimized = false;
    Renderer::OnWindowResize(e.get_width(), e.get_height());
    return false;
}
