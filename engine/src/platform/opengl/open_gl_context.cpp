//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/open_gl_context.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "x/core.h"
#include "x/core/x_log.h"

OpenGLContext::OpenGLContext(GLFWwindow *windowHandle) : m_windowHandle(windowHandle)
{
    X_CORE_ASSERT(windowHandle, "windowHandle is null");
}

void OpenGLContext::Init()
{
    glfwMakeContextCurrent(m_windowHandle);
    // 使用glad加载当前opengl版本的所有函数
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    X_CORE_ASSERT(status, "Could not load GLAD function");
    X_CORE_INFO("OpenGL Info:");
    X_CORE_INFO("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    X_CORE_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    X_CORE_INFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void OpenGLContext::SwapBuffers()
{
    glfwSwapBuffers(m_windowHandle);
}
