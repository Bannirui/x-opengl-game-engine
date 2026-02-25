//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"

class Window;
class Event;
class WindowCloseEvent;

class XApplication
{
public:
    XApplication();

    virtual ~XApplication();

    void Run();
    void OnEvent(Event& e);

private:
    bool onWindowClose(WindowCloseEvent& e);

private:
    std::unique_ptr<Window> m_window;
    bool                    m_running{true};
};

// To be defined in the CLIENT
XApplication *CreateApplication();
