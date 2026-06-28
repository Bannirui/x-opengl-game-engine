//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/core/input_codes.h"

class Input {
public:
    static bool IsKeyPressed(KeyCode keycode);
    static bool IsMouseButtonPressed(MouseCode button);
    static float GetMouseX();
    static float GetMouseY();
};
