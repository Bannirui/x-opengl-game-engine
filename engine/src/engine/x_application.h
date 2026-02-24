//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "engine/window.h"
#include "engine/core.h"

class XApplication
{
public:
    XApplication();

    virtual ~XApplication() = default;

    void Run();

private:
    std::unique_ptr<Window> m_window;
    bool                    m_running{false};
};

// To be defined in the CLIENT
XApplication *CreateApplication();
