//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "x/core/platform_detection.h"

#include <memory>
#include <string>

// 调试开关
#ifdef X_DEBUG
    #if defined(X_PLATFORM_WINDOWS)
        #define X_DEBUGBREAK() __debugbreak()
    #elif defined(X_PLATFORM_MAC) || defined(X_PLATFORM_LINUX)
        #include <csignal>
        #define X_DEBUGBREAK() raise(SIGTRAP)
    #else
        #error "Debugbreak not implemented for this platform"
    #endif
#else
    #define X_DEBUGBREAK()
#endif

// ---------- stringify ----------
#define X_EXPAND_MACRO(x) x
#define X_STRINGIFY_MACRO(x) #x
#define STR(x) X_STRINGIFY_MACRO(x)

#define BIT(x) (1 << x)

#ifndef X_GL_VERSION_MAJOR
    #define X_GL_VERSION_MAJOR 3
#endif

#ifndef X_GL_VERSION_MINOR
    #define X_GL_VERSION_MINOR 3
#endif

// ---------- GLSL version ----------
// OpenGL版本拼接成 #version 450 core
#define X_GL_VERSION_CORE "#version " STR(X_GL_VERSION_MAJOR) STR(X_GL_VERSION_MINOR) "0 core"

// ---------- Shader helper ----------
#define X_GLSL(src) X_GL_VERSION_CORE "\n" #src

namespace X {
    // 记录OpenGL的版本信息 运行时从机器上读 控制对GLAD的API调用和GLSL的语法版本选择
    struct GLRendererInfo {
        // 默认3.3
        int MajorVersion = 3;
        int MinorVersion = 3;
        // GLAD支持对spirv的支持 要借助shaderc把glsl编译成spirv字节码提交给GPU
        bool ARB_gl_spirv = false;

        bool IsAtLeast(int major, int minor) const {
            return MajorVersion > major || (MajorVersion == major && MinorVersion >= minor);
        }

        // 给GLSL的版本声明用 比如#version 330 core
        std::string GetGLSLVersionString() const {
            return "#version " + std::to_string(MajorVersion) + std::to_string(MinorVersion) + "0 core";
        }

        static GLRendererInfo& Get();
    };
}  // namespace X

namespace X {
    template <typename T>
    using Scope = std::unique_ptr<T>;

    template <typename T, typename... Args>
    Scope<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename... Args>
    Ref<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}  // namespace X

#include "x/core/x_assert.h"
#include "x/core/x_log.h"