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