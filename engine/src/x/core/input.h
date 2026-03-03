//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "pch.h"

#include <glm/glm.hpp>

#include "x/core/key_codes.h"
#include "x/core/mouse_codes.h"

class Input
{
public:
    static bool      IsKeyPressed(KeyCode keycode);
    static bool      IsMouseButtonPressed(MouseCode button);
    static glm::vec2 GetMousePos();
    static float     GetMouseX();
    static float     GetMouseY();
};
