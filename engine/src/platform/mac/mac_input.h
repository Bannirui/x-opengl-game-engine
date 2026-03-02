//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/core/input.h"

class MacInput : public Input
{
protected:
    bool                    isKeyPressedImpl(KeyCode keycode) override;
    bool                    isMouseButtonPressedImpl(MouseCode button) override;
    std::pair<float, float> getMousePosImpl() override;
    float                   getMouseXImpl() override;
    float                   getMouseYImpl() override;
};
