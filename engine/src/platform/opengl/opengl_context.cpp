//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/opengl_context.h"

#include "x/core/assert.h"
#include "x/core/base.h"
#include "x/core/log.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

X::GLRendererInfo& X::GLRendererInfo::Get() {
    static GLRendererInfo s_info;
    return s_info;
}

OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_windowHandle(windowHandle) {
    X_CORE_ASSERT(windowHandle, "windowHandle is null");
}

OpenGLContext::~OpenGLContext() {}

void OpenGLContext::Init() {
    X_PROFILE_FUNCTION();
    glfwMakeContextCurrent(m_windowHandle);
    // OpenGL的函数实现一部分由系统提供 一部分由显卡驱动提供 glfw把平台差异性封装好
    // 揉在一起告诉glad这些OpenGL的函数实现在哪儿
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    X_CORE_ASSERT(status, "Could not load GLAD function");
    X_CORE_INFO("OpenGL Info:");
    X_CORE_INFO("  Vendor: {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    X_CORE_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    X_CORE_INFO("  Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    int versionMajor;
    int versionMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

    X_CORE_ASSERT(versionMajor > 3 || (versionMajor == 3 && versionMinor >= 3),
                  "Requires at least OpenGL version 3.3, not support {}.{}", versionMajor, versionMinor);

    // 拿到运行时的OpenGL版本缓存起来
    auto& info = X::GLRendererInfo::Get();
    info.MajorVersion = versionMajor;
    info.MinorVersion = versionMinor;

#ifdef glSpecializeShaderARB
    // 因为要用Shaderc编译GLSL成spirv 看看现在OpenGL版本支持不支持
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLint i = 0; i < numExtensions; ++i) {
        const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (strcmp(extension, "GL_ARB_gl_spirv") == 0) {
            info.ARB_gl_spirv = true;
            break;
        }
    }
#endif

    X_CORE_INFO("  GL version: {}.{}, ARB_gl_spirv: {}", info.MajorVersion, info.MinorVersion, info.ARB_gl_spirv);
}

void OpenGLContext::SwapBuffers() {
    X_PROFILE_FUNCTION();
    glfwSwapBuffers(m_windowHandle);
}
