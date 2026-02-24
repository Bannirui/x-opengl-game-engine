//
// Created by dingrui on 2/23/26.
//

#include "engine/x_application.h"

#include "engine/x_log.h"
#include "engine/events/application_event.h"

XApplication::XApplication()
{
    m_window  = std::unique_ptr<Window>(Window::Create());
    m_running = true;
}

void XApplication::Run()
{
    while (m_running)
    {
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        m_window->OnUpdate();
    }
}
