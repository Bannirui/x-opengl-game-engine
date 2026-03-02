//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "pch.h"

#include "x/core/core.h"
#include "x/core/key_codes.h"
#include "x/core/mouse_codes.h"

class Input
{
protected:
    Input() = default;

public:
    static X::Scope<Input> Create();

    virtual ~Input() = default;

    // 禁止掉拷贝构造 在Create函数中要用移动构造
    Input(const Input &)            = delete;
    Input &operator=(const Input &) = delete;

    static bool IsKeyPressed(KeyCode keycode) { return s_instance->isKeyPressedImpl(keycode); }
    static bool IsMouseButtonPressed(MouseCode button) { return s_instance->isMouseButtonPressedImpl(button); }
    static std::pair<float, float> GetMousePos() { return s_instance->getMousePosImpl(); }
    static float                   GetMouseX() { return s_instance->getMouseXImpl(); }
    static float                   GetMouseY() { return s_instance->getMouseYImpl(); }

protected:
    virtual bool                    isKeyPressedImpl(KeyCode keycode)          = 0;
    virtual bool                    isMouseButtonPressedImpl(MouseCode button) = 0;
    virtual std::pair<float, float> getMousePosImpl()                          = 0;
    virtual float                   getMouseXImpl()                            = 0;
    virtual float                   getMouseYImpl()                            = 0;

private:
    static X::Scope<Input> s_instance;
};
