//
// Created by rui ding on 2026/2/24.
//

#include "platform/mac/mac_window.h"

#include "platform/opengl/opengl_context.h"
#include "x/core/assert.h"
#include "x/core/log.h"
#include "x/events/application_event.h"
#include "x/events/key_event.h"
#include "x/events/mouse_event.h"

static uint8_t s_GLFWWindowCount = 0;

static void glfwErrorCallback(int error, const char* description) {
    CORE_ERROR("GLFW error ({0}): {1}", error, description);
}

MacWindow::MacWindow(const WindowProps& props) {
    X_PROFILE_FUNCTION();
    init(props);
}

MacWindow::~MacWindow() {
    X_PROFILE_FUNCTION();
    shutdown();
}

void MacWindow::OnUpdate() {
    X_PROFILE_FUNCTION();
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

void MacWindow::SetVSync(bool enabled) {
    X_PROFILE_FUNCTION();
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    m_data.vSync = enabled;
}

void MacWindow::init(const WindowProps& props) {
    X_PROFILE_FUNCTION();
    m_data.title = props.title;
    m_data.width = props.width;
    m_data.height = props.height;

    CORE_INFO("Create window {0} ({1}, {2})", props.title, props.width, props.height);

    if (s_GLFWWindowCount == 0) {
        X_PROFILE_SCOPE("glfwInit");
        int succ = glfwInit();
        CORE_ASSERT(succ, "Could not init GLFW");
        glfwSetErrorCallback(glfwErrorCallback);
    }
    // 告诉glfw我要用的OpenGL版本是 让它在创建OpenGL上下文的时候能正确创建对应的版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    {
        X_PROFILE_SCOPE("glfwCreateWindow");
        m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title.c_str(),
                                    nullptr, nullptr);
        ++s_GLFWWindowCount;
    }
    CORE_ASSERT(m_window, "Failed to create GLFW window");

    glfwMakeContextCurrent(m_window);

    m_context = GraphicsContext::Create(m_window);
    // 把OpenGL的函数实现地址告诉glad
    m_context->Init();

    glfwSetWindowUserPointer(m_window, this);

    SetVSync(true);
    // glfw的回调注册
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        self->m_data.width = width;
        self->m_data.height = height;
        WindowResizeEvent event(width, height);
        self->m_data.eventCallback(event);
    });
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        self->m_data.eventCallback(event);
    });
    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        switch (action) {
            case GLFW_PRESS: {
                KeyPressEvent event(static_cast<KeyCode>(key), false);
                self->m_data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleaseEvent event(static_cast<KeyCode>(key));
                self->m_data.eventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressEvent event(static_cast<KeyCode>(key), true);
                self->m_data.eventCallback(event);
                break;
            }
        }
    });
    glfwSetCharCallback(m_window, [](GLFWwindow* window, uint32_t keycode) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        KeyTypedEvent event(static_cast<KeyCode>(keycode));
        self->m_data.eventCallback(event);
    });
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                self->m_data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                self->m_data.eventCallback(event);
                break;
            }
        }
    });
    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(xOffset, yOffset);
        self->m_data.eventCallback(event);
    });
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
        auto* self = static_cast<MacWindow*>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(xPos, yPos);
        self->m_data.eventCallback(event);
    });
}

void MacWindow::shutdown() {
    X_PROFILE_FUNCTION();
    glfwDestroyWindow(m_window);
    --s_GLFWWindowCount;
    if (s_GLFWWindowCount == 0) {
        CORE_INFO("Terminating GLFW window");
        glfwTerminate();
    }
}
