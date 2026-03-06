//
// Created by rui ding on 2026/2/26.
//

#ifdef __linux__

#include "x/core/input.h"

#include "x/window.h"
#include "x/core/x_application.h"

bool Input::IsKeyPressed(KeyCode keycode)
{
    auto window = static_cast<GLFWwindow*>(XApplication::Get().get_window().get_nativeWindow());
    int  state  = glfwGetKey(window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button)
{
    auto window = static_cast<GLFWwindow*>(XApplication::Get().get_window().get_nativeWindow());
    int  state  = glfwGetMouseButton(window, button);
    return state == GLFW_PRESS;
}

glm::vec2 Input::GetMousePos()
{
    auto   window = static_cast<GLFWwindow*>(XApplication::Get().get_window().get_nativeWindow());
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

float Input::GetMouseX()
{
    return GetMousePos().x;
}

float Input::GetMouseY()
{
    return GetMousePos().y;
}

#endif
