//
// Created by rui ding on 2026/2/26.
//

#ifdef __linux__

    #include "x/core/application.h"
    #include "x/core/input.h"
    #include "x/window.h"

    #include <glm/glm.hpp>

// 拿到当前鼠标的位置坐标(x,y)
static std::pair<float, float> getMousePos() {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

bool Input::IsKeyPressed(KeyCode keycode) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetKey(window, static_cast<int>(keycode));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetMouseButton(window, static_cast<int>(button));
    return state == GLFW_PRESS;
}

float Input::GetMouseX() {
    auto [x, y] = getMousePos();
    return x;
}

float Input::GetMouseY() {
    auto [x, y] = getMousePos();
    return y;
}

#endif
