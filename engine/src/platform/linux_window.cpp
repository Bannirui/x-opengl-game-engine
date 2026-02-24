//
// Created by rui ding on 2026/2/24.
//

#include "platform/linux_window.h"

#include "engine/x_log.h"

static bool s_GLFWInitialized = false;

LinuxWindow::LinuxWindow(const WindowProps &props)
{
    init(props);
}

LinuxWindow::~LinuxWindow()
{
    shutdown();
}

void LinuxWindow::OnUpdate()
{
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

void LinuxWindow::SetVSync(bool enabled)
{
    if (enabled)
    {
        glfwSwapInterval(1);
    }
    else
    {
        glfwSwapInterval(0);
    }
    m_data.vSync = enabled;
}

void LinuxWindow::init(const WindowProps &props)
{
    m_data.title  = props.title;
    m_data.width  = props.width;
    m_data.height = props.height;

    X_CORE_INFO("Create window {0} ({1}, {2})", props.title, props.width, props.height);

    if (!s_GLFWInitialized)
    {
        int succ = glfwInit();
        X_CORE_ASSERT(succ, "Could not init GLFW");
        s_GLFWInitialized = true;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), props.title.c_str(),
                                nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, &m_data);
    // 使用glad加载当前opengl版本的所有函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        X_CORE_ASSERT(false, "Could not load GLAD function");
    }
    SetVSync(true);
}

void LinuxWindow::shutdown()
{
    glfwDestroyWindow(m_window);
}
