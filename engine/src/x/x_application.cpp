//
// Created by dingrui on 2/23/26.
//

#include "x/x_application.h"

#include "layer.h"
#include "x/x_log.h"
#include "x/events/event.h"
#include "x/events/application_event.h"
#include "x/window.h"

// 类的成员函数x绑定成可以直接调用的对象
#define BIND_EVENT_FN(x) std::bind(&XApplication::x, this, std::placeholders::_1)

XApplication::XApplication()
{
    m_window = std::unique_ptr<Window>(Window::Create());
    m_window->SetEventCallback(BIND_EVENT_FN(OnEvent));
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
        m_window->OnUpdate();
    }
}

void XApplication::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
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
