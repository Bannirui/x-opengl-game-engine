//
// Created by rui ding on 2026/2/24.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "pch.h"
#include "x/core/base.h"

class Event;

struct WindowProps
{
    std::string title;
    uint32_t    width;
    uint32_t    height;

    WindowProps(const std::string& title = "X-OpenGL-GAME-ENGINE", uint32_t width = 1600, uint32_t height = 900)
        : title(title), width(width), height(height)
    {
    }
};

// 封装的窗体 掩藏glfw的细节
class Window
{
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth() const  = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled)                            = 0;
    virtual bool IsVSync()                                         = 0;

    // 暴露glfw的窗体
    virtual void* get_nativeWindow() const = 0;

    static X::Scope<Window> Create(const WindowProps& props = WindowProps());

protected:
    Window() = default;
};