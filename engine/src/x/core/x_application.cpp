//
// Created by dingrui on 2/23/26.
//

#include "x/core/x_application.h"

#include "x/renderer/renderer.h"
#include "x/core/core.h"
#include "x/core/layer.h"
#include "x/core/x_log.h"
#include "x/events/application_event.h"
#include "x/window.h"
#include "x/core/input.h"
#include "x/imgui/im_gui_layer.h"
#include "x/core/timestep.h"

XApplication *XApplication::s_instance = nullptr;

XApplication::XApplication()
{
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    m_window   = Window::Create();
    m_window->SetEventCallback([this](Event &e) { this->OnEvent(e); });

    Renderer::Init();

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);
}

XApplication::~XApplication()
{
    Renderer::Shutdown();
}

void XApplication::Run()
{
    while (m_running)
    {
        float    time     = static_cast<float>(glfwGetTime());
        Timestep timestep = time - m_lastFrameTime;
        m_lastFrameTime   = time;

        if (!m_minimized)
        {
            for (Layer *layer : m_layerStack)
            {
                layer->OnUpdate(timestep);
            }
        }

        m_ImGuiLayer->Begin();
        for (Layer *layer : m_layerStack)
        {
            layer->OnImguiRender();
        }
        m_ImGuiLayer->End();

        m_window->OnUpdate();
    }
}

void XApplication::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &e) { return this->onWindowClose(e); });
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) { return this->onWindowResize(e); });
    for (auto it = m_layerStack.end(); it != m_layerStack.begin();)
    {
        (*--it)->OnEvent(e);
        if (e.Handled)
        {
            break;
        }
    }
}

void XApplication::PushLayer(Layer *layer)
{
    m_layerStack.PushLayer(layer);
}

void XApplication::PushOverlay(Layer *layer)
{
    m_layerStack.PushOverlay(layer);
}

bool XApplication::onWindowClose(WindowCloseEvent &e)
{
    m_running = false;
    return true;
}

bool XApplication::onWindowResize(WindowResizeEvent &e)
{
    if (e.get_width() == 0 || e.get_height() == 0)
    {
        m_minimized = true;
        return false;
    }
    m_minimized = false;
    Renderer::OnWindowResize(e.get_width(), e.get_height());
    return false;
}
