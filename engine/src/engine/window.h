//
// Created by rui ding on 2026/2/24.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "pch.h"
#include "engine/events/event.h"

struct WindowProps
{
    std::string title;
    uint32_t    width;
    uint32_t    height;

    WindowProps(const std::string &title = "X-OpenGL-GAME-ENGINE", uint32_t width = 1280, uint32_t height = 720)
        : title(title), width(width), height(height)
    {
    }
};

class Window
{
public:
    using EventCallbackFn = std::function<void(Event &)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth() const  = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn &callback) = 0;
    virtual void SetVSync(bool enabled)                            = 0;
    virtual bool IsVSync()                                         = 0;

    static Window *Create(const WindowProps &props = WindowProps());
};
