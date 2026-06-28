//
// Created by rui ding on 2026/2/24.
//

#pragma once

#include "x/core/base.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>

class Event;

struct WindowProps {
    std::string title;
    uint32_t width;
    uint32_t height;

    WindowProps(const std::string& title = "X-OpenGL-GAME-ENGINE", uint32_t width = 1600, uint32_t height = 900)
        : title(title), width(width), height(height) {}
};

// 封装的窗体 掩藏glfw的细节
class Window {
public:
    // glfw的窗体事件封装成自己的事件后交给这个函数 这个汉书汉书函数负责对政治体制事件的处理
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

    /**
     * 缓冲区刷新的频率保持与显示器刷新率同步 这样可以消除画面撕裂
     */
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() = 0;

    // 暴露glfw的窗体
    virtual void* get_nativeWindow() const = 0;

    /**
     * 封装平台差异性
     */
    static Scope<Window> Create(const WindowProps& props = WindowProps());

protected:
    Window() = default;
};