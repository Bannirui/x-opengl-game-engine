//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"
#include "x/layer_stack.h"

class ImGuiLayer;
class Window;
class Event;
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

    inline Window              &get_window() const { return *m_window; }
    inline static XApplication &Get() { return *s_instance; }

private:
    bool onWindowClose(WindowCloseEvent &e);

private:
    std::unique_ptr<Window> m_window;
    bool                    m_running{true};
    LayerStack              m_layerStack;
    ImGuiLayer             *m_ImGuiLayer;

private:
    static XApplication *s_instance;
};

// To be defined in the CLIENT
XApplication *CreateApplication();
