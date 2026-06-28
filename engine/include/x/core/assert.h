//
// Created by rui ding on 2026/3/4.
//

#pragma once

#include "x/core/base.h"
#include "x/core/log.h"

#include <filesystem>

#ifdef X_DEBUG
    #define X_ENABLE_ASSERTS
#endif

// 没有开启调试开关的时候支持无参数
#ifdef X_ENABLE_ASSERTS
    #define X_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
        {                                                 \
            if (!(check)) {                               \
                X##type##ERROR(msg, __VA_ARGS__);         \
                X_DEBUGBREAK();                           \
            }                                             \
        }
    #define X_INTERNAL_ASSERT_WITH_MSG(type, check, msg, ...)                                                 \
        {                                                                                                     \
            if (!(check)) {                                                                                   \
                X##type##ERROR("[{}:{}] " msg, std::filesystem::path(__FILE__).filename().string(), __LINE__, \
                               ##__VA_ARGS__);                                                                \
                X_DEBUGBREAK();                                                                               \
            }                                                                                                 \
        }
    #define X_INTERNAL_ASSERT_NO_MSG(type, check)                                                          \
        X_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", X_STRINGIFY_MACRO(check), \
                               std::filesystem::path(__FILE__).filename().string(), __LINE__)

    // 参数个数计数 最多9个
    #define X_ASSERT_ARG_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
    #define X_ASSERT_ARG_N(...) X_EXPAND_MACRO(X_ASSERT_ARG_COUNT(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

    // 安全的token拼接 先展开参数再拼接
    #define X_ASSERT_CAT_IMPL(a, b) a##b
    #define X_ASSERT_CAT(a, b) X_ASSERT_CAT_IMPL(a, b)

    // N=1->NO_MSG N>=2->WITH_MSG
    #define X_ASSERT_1(check) X_INTERNAL_ASSERT_NO_MSG(_, check)
    #define X_ASSERT_2(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_3(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_4(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_5(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_6(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_7(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_8(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define X_ASSERT_9(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)

    #define X_CORE_ASSERT_1(check) X_INTERNAL_ASSERT_NO_MSG(_CORE_, check)
    #define X_CORE_ASSERT_2(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_3(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_4(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_5(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_6(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_7(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_8(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)
    #define X_CORE_ASSERT_9(check, msg, ...) X_INTERNAL_ASSERT_WITH_MSG(_CORE_, check, msg, ##__VA_ARGS__)

    // 展开后触发两轮宏替换 第一轮X_CAT拼出X_ASSERT_N 第二轮X_ASSERT_N展开为断言体 支持格式化字符串带变量
    #define X_ASSERT(...) X_EXPAND_MACRO(X_ASSERT_CAT(X_ASSERT_, X_ASSERT_ARG_N(__VA_ARGS__))(__VA_ARGS__))
    #define X_CORE_ASSERT(...) X_EXPAND_MACRO(X_ASSERT_CAT(X_CORE_ASSERT_, X_ASSERT_ARG_N(__VA_ARGS__))(__VA_ARGS__))
#else
    #define X_ASSERT(x, ...) ((void)(x))
    #define X_CORE_ASSERT(x, ...) ((void)(x))
#endif
