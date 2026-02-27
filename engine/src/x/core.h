//
// Created by dingrui on 2/23/26.
//

#pragma once

#define BIT(x) (1 << x)

// ================================
// Platform detection
// ================================
#if defined(_WIN32) || defined(_WIN64)
    #define X_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #define X_PLATFORM_MAC
#elif defined(__linux__)
    #define X_PLATFORM_LINUX
#else
    #error "Unknown platform!"
#endif

// ================================
// Debug Break (跨平台实现)
// ================================
#if defined(X_PLATFORM_WINDOWS)
    #define X_DEBUGBREAK() __debugbreak()
#elif defined(X_PLATFORM_MAC) || defined(X_PLATFORM_LINUX)
    #include <csignal>
    #define X_DEBUGBREAK() raise(SIGTRAP)
#else
    #error "Debugbreak not implemented for this platform"
#endif

// 调试开关
#ifdef X_DEBUG
	#define X_ENABLE_ASSERTS
#endif

// 没有开启开关的时候支持无参数
#ifdef X_ENABLE_ASSERTS
    #define X_ASSERT(x, ...) \
        { if(!(x)) { X_ERROR("Assertion Failed: {0}", __VA_ARGS__); X_DEBUGBREAK(); } }
    #define X_CORE_ASSERT(x, ...) \
        { if(!(x)) { X_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); X_DEBUGBREAK(); } }
#else
    #define X_ASSERT(x, ...) ((void)(x))
    #define X_CORE_ASSERT(x, ...) ((void)(x))
#endif

// ---------- stringify ----------
#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

#ifndef X_GL_VERSION_MAJOR
#define X_GL_VERSION_MAJOR 3
#endif

#ifndef X_GL_VERSION_MINOR
#define X_GL_VERSION_MINOR 3
#endif

// ---------- GLSL version ----------
// 计算：major * 100 + minor * 10
#define X_GLSL_VERSION_NUM \
    (X_GL_VERSION_MAJOR * 100 + X_GL_VERSION_MINOR * 10)

// 强制展开一次
#define X_EXPAND(x) x
#define X_GLSL_VERSION X_EXPAND(X_GLSL_VERSION_NUM)

// ---------- GLSL header ----------
// OpenGL版本拼接成 #version 450 core
#define X_GL_VERSION_CORE \
    "#version " STR(X_GL_VERSION_MAJOR) STR(X_GL_VERSION_MINOR) "0 core"

// ---------- Shader helper ----------
#define X_GLSL(src) \
    X_GL_VERSION_CORE "\n" \
    #src
