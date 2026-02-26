//
// Created by dingrui on 2/23/26.
//

#pragma once

#define BIT(x) (1 << x)

// 没有开启开关的时候支持无参数
#ifdef X_ENABLE_ASSERTS
    #define X_ASSERT(x, ...) { if(!(x)) { X_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define X_CORE_ASSERT(x, ...) { if(!(x)) { X_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define X_ASSERT(x, ...) ((void)(x))
    #define X_CORE_ASSERT(x, ...) ((void)(x))
#endif

// ---------- stringify ----------
#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)

// ---------- token concatenation ----------
#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

// ---------- GLSL version ----------
// OpenGL版本拼接成 #version 450 core
#define X_GL_VERSION \
"#version " STR(CAT(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)0) " core"