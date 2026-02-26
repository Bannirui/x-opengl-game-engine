//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/input.h"

class MacInput : public Input
{
protected:
    bool                    isKeyPressedImpl(int keycode) override;
    bool                    isMouseButtonPressedImpl(int button) override;
    std::pair<float, float> getMousePosImpl() override;
    float                   getMouseXImpl() override;
    float                   getMouseYImpl() override;
};
