//
// Created by rui ding on 2026/2/24.
//

#pragma once

#include "x/core/base.h"
#include "x/window.h"

class GraphicsContext;

class LinuxWindow : public Window
{
public:
    LinuxWindow(const WindowProps &props);
    virtual ~LinuxWindow();

    void OnUpdate() override;

    uint32_t GetWidth() const override { return m_data.width; }
    uint32_t GetHeight() const override { return m_data.height; }
    void     SetEventCallback(const EventCallbackFn &callback) override { m_data.eventCallback = callback; }
    void     SetVSync(bool enabled) override;
    bool     IsVSync() override { return m_data.vSync; }

    void *get_nativeWindow() const override { return m_window; }

private:
    virtual void init(const WindowProps &props);
    virtual void shutdown();

private:
    struct WindowData
    {
        std::string     title;
        uint32_t        width;
        uint32_t        height;
        bool            vSync;
        EventCallbackFn eventCallback =[](Event &) { }; // it will be called later, set an empty callback for protect
    };

    WindowData       m_data;
    GLFWwindow      *m_window;
    X::Scope<GraphicsContext> m_context;
};
