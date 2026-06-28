//
// Created by rui ding on 2026/2/26.
//

#ifdef __linux__

    #include "x/core/application.h"
    #include "x/core/input.h"
    #include "x/window.h"

bool Input::IsKeyPressed(KeyCode keycode) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetKey(window, keycode);
    return state == GLFW_PRESS;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    int state = glfwGetMouseButton(window, button);
    return state == GLFW_PRESS;
}

float Input::GetMouseX() {
    return GetMousePos().x;
}

float Input::GetMouseY() {
    return GetMousePos().y;
}

glm::vec2 Input::GetMousePos() {
    auto window = static_cast<GLFWwindow*>(Application::Get().get_window().get_nativeWindow());
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}
#endif
