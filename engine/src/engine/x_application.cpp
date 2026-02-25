//
// Created by dingrui on 2/23/26.
//

#include "engine/x_application.h"

#include "engine/x_log.h"
#include "engine/events/application_event.h"
#include "engine/window.h"

// 类的成员函数x绑定成可以直接调用的对象
#define BIND_EVENT_FN(x) std::bind(&XApplication::x, this, std::placeholders::_1)

XApplication::XApplication()
{
    m_window  = std::unique_ptr<Window>(Window::Create());
    m_window->SetEventCallback(BIND_EVENT_FN(OnEvent));
}

XApplication::~XApplication() {}

void XApplication::Run()
{
    while (m_running)
    {
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        m_window->OnUpdate();
    }
}

void XApplication::OnEvent(Event &e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
    X_TRACE("{0}", e);
}

bool XApplication::onWindowClose(WindowCloseEvent &e)
{
    m_running = false;
    return true;
}
