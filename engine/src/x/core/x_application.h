//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "pch.h"
#include "x/layer_stack.h"

class WindowResizeEvent;
class ImGuiLayer;
class Window;
class Event;
class WindowCloseEvent;
class Layer;

int main(int argc, char** argv);

struct ApplicationCommandLineArgs
{
    int    Count = 0;
    char** Args  = nullptr;

    const char* operator[](int index) const
    {
        X_CORE_ASSERT(index < Count, "Index out of bounds");
        return Args[index];
    }
};

class XApplication
{
public:
    XApplication(const std::string& name = "X APP", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
    virtual ~XApplication();

    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* layer);

    void Close();

    Window& get_window() const { return *m_window; }

    ImGuiLayer* get_ImGuiLayer() const { return m_ImGuiLayer; }

    static XApplication& Get() { return *s_instance; }

    ApplicationCommandLineArgs get_commandLineArgs() const { return m_commandLineArgs; }

private:
    void run();
    bool onWindowClose(WindowCloseEvent& e);
    bool onWindowResize(WindowResizeEvent& e);

private:
    ApplicationCommandLineArgs m_commandLineArgs;
    X::Scope<Window>           m_window;
    bool                       m_running{true};
    LayerStack                 m_layerStack;
    ImGuiLayer*                m_ImGuiLayer;
    float                      m_lastFrameTime{0.0f};
    bool                       m_minimized{false};  // 窗口最小化

private:
    static XApplication* s_instance;
    friend int           main(int argc, char** argv);
};

// To be defined in the CLIENT
XApplication* CreateApplication(ApplicationCommandLineArgs args);
