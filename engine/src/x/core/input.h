//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "pch.h"

#include "x/core.h"

class Input
{
public:
    static void Create();

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    inline static bool IsKeyPressed(int keycode) { return s_instance->isKeyPressedImpl(keycode); }
    inline static bool IsMouseButtonPressed(int button) { return s_instance->isMouseButtonPressedImpl(button); }
    inline static std::pair<float, float> GetMousePos() { return s_instance->getMousePosImpl(); }
    inline static float                   GetMouseX() { return s_instance->getMouseXImpl(); }
    inline static float                   GetMouseY() { return s_instance->getMouseYImpl(); }
protected:
    Input() = default;

    virtual bool                    isKeyPressedImpl(int keycode)        = 0;
    virtual bool                    isMouseButtonPressedImpl(int button) = 0;
    virtual std::pair<float, float> getMousePosImpl()                    = 0;
    virtual float                   getMouseXImpl()                      = 0;
    virtual float                   getMouseYImpl()                      = 0;

private:
    static X::Scope<Input> s_instance;
};
