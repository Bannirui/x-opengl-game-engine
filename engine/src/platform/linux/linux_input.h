//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "../../x/core/input.h"

class LinuxInput : public Input
{
protected:
    bool                    isKeyPressedImpl(int keycode) override;
    bool                    isMouseButtonPressedImpl(int button) override;
    std::pair<float, float> getMousePosImpl() override;
    float                   getMouseXImpl() override;
    float                   getMouseYImpl() override;
};
