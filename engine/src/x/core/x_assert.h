//
// Created by rui ding on 2026/3/4.
//

#pragma once

#include "x/core/base.h"
#include "x/core/x_log.h"
#include <filesystem>

// 没有开启调试开关的时候支持无参数
#ifdef X_ENABLE_ASSERTS
// Alternatively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the
// default message
#    define X_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
        {                                                 \
            if (!(check))                                 \
            {                                             \
                X##type##ERROR(msg, __VA_ARGS__);         \
                X_DEBUGBREAK();                           \
            }                                             \
        }
#    define X_INTERNAL_ASSERT_WITH_MSG(type, check, ...) \
        X_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#    define X_INTERNAL_ASSERT_NO_MSG(type, check)                                                          \
        X_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", X_STRINGIFY_MACRO(check), \
                               std::filesystem::path(__FILE__).filename().string(), __LINE__)

#    define X_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#    define X_INTERNAL_ASSERT_GET_MACRO(...) \
        X_EXPAND_MACRO(                      \
            X_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, X_INTERNAL_ASSERT_WITH_MSG, X_INTERNAL_ASSERT_NO_MSG))

// Currently accepts at least the condition and one additional parameter (the message) being optional
#    define X_ASSERT(...) X_EXPAND_MACRO(X_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))
#    define X_CORE_ASSERT(...) X_EXPAND_MACRO(X_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__))
#else
#    define X_ASSERT(x, ...) ((void)(x))
#    define X_CORE_ASSERT(x, ...) ((void)(x))
#endif