//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"
#include "engine/events/event.h"
#include "engine/layer_stack.h"

class Window;
class WindowCloseEvent;
class Layer;

class XApplication
{
public:
    XApplication();

    virtual ~XApplication();

    void Run();
    void OnEvent(Event &e);

    void PushLayer(Layer *layer);
    void PushOverlay(Layer *layer);

private:
    bool onWindowClose(WindowCloseEvent &e);

private:
    std::unique_ptr<Window> m_window;
    bool                    m_running{true};
    LayerStack              m_layerStack;
};

// To be defined in the CLIENT
XApplication *CreateApplication();
