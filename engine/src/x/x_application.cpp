//
// Created by dingrui on 2/23/26.
//

#include "x/x_application.h"

#include "x/core.h"
#include "x/layer.h"
#include "x/x_log.h"
#include "x/events/application_event.h"
#include "x/window.h"
#include "x/input.h"
#include "x/imgui/im_gui_layer.h"

XApplication *XApplication::s_instance = nullptr;

XApplication::XApplication()
{
    X_CORE_ASSERT(!s_instance, "Application already exists");
    s_instance = this;
    m_window   = std::unique_ptr<Window>(Window::Create());
    m_window->SetEventCallback(X_BIND_EVENT_FN(XApplication::OnEvent));
    // todo 创建input
    Input::Create();

    m_ImGuiLayer = new ImGuiLayer();
    PushOverlay(m_ImGuiLayer);
}

XApplication::~XApplication() {}

void XApplication::Run()
{
    while (m_running)
    {
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        for (Layer *layer : m_layerStack)
        {
            layer->OnUpdate();
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
    dispatcher.Dispatch<WindowCloseEvent>(X_BIND_EVENT_FN(XApplication::onWindowClose));
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
