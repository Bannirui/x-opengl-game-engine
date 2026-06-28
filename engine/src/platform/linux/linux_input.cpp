//
// Created by rui ding on 2026/2/26.
//

#ifdef __linux__

    #include "x/core/application.h"
    #include "x/core/input.h"
    #include "x/window.h"

    #include <glm/glm.hpp>

static glm::vec2 getMousePos() {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

bool Input::IsKeyPressed(KeyCode keycode) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetKey(window, static_cast<int>(keycode));
    return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetMouseButton(window, static_cast<int>(button));
    return state == GLFW_PRESS;
}

float Input::GetMouseX() {
    return getMousePos().x;
}

float Input::GetMouseY() {
    return getMousePos().y;
}

#endif
