//
// Created by rui ding on 2026/2/24.
//

#include "platform/linux/linux_window.h"

#include "platform/opengl/opengl_context.h"
#include "x/core/assert.h"
#include "x/core/log.h"
#include "x/events/application_event.h"
#include "x/events/key_event.h"
#include "x/events/mouse_event.h"

static uint8_t s_GLFWWindowCount = 0;

// 注册给glfw glfw发生异常时会回调这个函数
static void glfwErrorCallback(int error, const char* description) {
    X_CORE_ERROR("GLFW error ({0}): {1}", error, description);
}

LinuxWindow::LinuxWindow(const WindowProps& props) {
    X_PROFILE_FUNCTION();
    init(props);
}

LinuxWindow::~LinuxWindow() {
    X_PROFILE_FUNCTION();
    shutdown();
}

void LinuxWindow::OnUpdate() {
    X_PROFILE_FUNCTION();
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

void LinuxWindow::SetVSync(bool enabled) {
    X_PROFILE_FUNCTION();
    if (enabled) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    m_data.vSync = enabled;
}

void LinuxWindow::init(const WindowProps& props) {
    X_PROFILE_FUNCTION();
    m_data.title = props.title;
    m_data.width = props.width;
    m_data.height = props.height;

    X_CORE_INFO("Create window {0} ({1}, {2})", props.title, props.width, props.height);

    if (s_GLFWWindowCount == 0) {
        X_PROFILE_SCOPE("glfwInit");
        int succ = glfwInit();
        X_CORE_ASSERT(succ, "Could not init GLFW");
        glfwSetErrorCallback(glfwErrorCallback);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    {
        X_PROFILE_SCOPE("glfwCreateWindow");
        m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title.c_str(),
                                    nullptr, nullptr);
        ++s_GLFWWindowCount;
    }
    X_CORE_ASSERT(m_window, "Failed to create GLFW window");

    glfwMakeContextCurrent(m_window);

    m_context = GraphicsContext::Create(m_window);
    m_context->Init();

    // 把封装的window放到glfw上下文 有glfw窗体事件的时候再把事件转发给自己的window处理
    glfwSetWindowUserPointer(m_window, this);

    SetVSync(true);
    // glfw的回调注册
    this->registerWindowCallbacks();
}

void LinuxWindow::shutdown() {
    X_PROFILE_FUNCTION();
    glfwDestroyWindow(m_window);
    --s_GLFWWindowCount;
    if (s_GLFWWindowCount == 0) {
        X_CORE_INFO("Terminating GLFW window");
        glfwTerminate();
    }
}

void LinuxWindow::registerWindowCallbacks() const {
    // 窗口大小
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
        self->m_data.width = width;
        self->m_data.height = height;
        WindowResizeEvent event(width, height);
        self->m_data.eventCallback(event);
    });
    // 关闭窗口
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        self->m_data.eventCallback(event);
    });
    // 键盘按键
    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
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
            default:
                break;
        }
    });
    // 键盘按键
    glfwSetCharCallback(m_window, [](GLFWwindow* window, uint32_t keycode) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
        KeyTypedEvent event(static_cast<KeyCode>(keycode));
        self->m_data.eventCallback(event);
    });
    // 鼠标点击
    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
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
            default:
                break;
        }
    });
    // 鼠标滚轮
    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        self->m_data.eventCallback(event);
    });
    // 鼠标拖动
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
        auto* self = static_cast<LinuxWindow*>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
        self->m_data.eventCallback(event);
    });
}
