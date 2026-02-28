//
// Created by rui ding on 2026/2/24.
//

#pragma once

#include "x/core.h"
#include "x/window.h"

class GraphicsContext;

class MacWindow : public Window
{
public:
    MacWindow(const WindowProps &props);
    virtual ~MacWindow();

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
        std::string title;
        uint32_t    width;
        uint32_t    height;
        bool        vSync;
        // 回调函数 用来传播glfw的事件
        EventCallbackFn eventCallback;
    };

    WindowData       m_data;
    GLFWwindow      *m_window;
    X::Scope<GraphicsContext> m_context;
};
