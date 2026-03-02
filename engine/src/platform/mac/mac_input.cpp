//
// Created by rui ding on 2026/2/26.
//

#include "mac_input.h"

#include "x/window.h"
#include "x/core/x_application.h"

bool MacInput::isKeyPressedImpl(KeyCode keycode)
{
    auto window = static_cast<GLFWwindow *>(XApplication::Get().get_window().get_nativeWindow());
    int  state  = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool MacInput::isMouseButtonPressedImpl(MouseCode button)
{
    auto window = static_cast<GLFWwindow *>(XApplication::Get().get_window().get_nativeWindow());
    int  state  = glfwGetMouseButton(window, button);
    return state == GLFW_PRESS;
}

std::pair<float, float> MacInput::getMousePosImpl()
{
    auto   window = static_cast<GLFWwindow *>(XApplication::Get().get_window().get_nativeWindow());
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

float MacInput::getMouseXImpl()
{
    auto [x, y] = getMousePosImpl();
    return x;
}

float MacInput::getMouseYImpl()
{
    auto [x, y] = getMousePosImpl();
    return y;
}
