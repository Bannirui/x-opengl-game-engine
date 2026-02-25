//
// Created by rui ding on 2026/2/24.
//

#pragma once
#include <cstdint>

using MouseCode = uint16_t;

// 鼠标按键编码
namespace MOUSE
{
    enum : MouseCode
    {
        // From glfw3.h
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        ButtonLast   = Button7,
        ButtonLeft   = Button0,
        ButtonRight  = Button1,
        ButtonMiddle = Button2
    };
}