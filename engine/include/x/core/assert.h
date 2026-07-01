//
// Created by rui ding on 2026/3/4.
//

#pragma once

#include "x/core/base.h"
#include "x/log/log.h"

#include <filesystem>

#ifdef X_DEBUG
    #define X_ENABLE_ASSERTS
#endif

// 没有开启调试开关的时候支持无参数
#ifdef X_ENABLE_ASSERTS
    #define INTERNAL_ASSERT_IMPL(type, check, msg, ...)   \
        {                                                 \
            if (!(check)) {                               \
                type##ERROR(msg, __VA_ARGS__);            \
                DEBUGBREAK();                             \
            }                                             \
        }
    #define INTERNAL_ASSERT_WITH_MSG(type, check, msg, ...)                                                   \
        {                                                                                                     \
            if (!(check)) {                                                                                   \
                type##ERROR("[{}:{}] " msg, std::filesystem::path(__FILE__).filename().string(), __LINE__,    \
                               ##__VA_ARGS__);                                                                \
                DEBUGBREAK();                                                                                 \
            }                                                                                                 \
        }
    #define INTERNAL_ASSERT_NO_MSG(type, check)                                                               \
        INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", STRINGIFY_MACRO(check),        \
                               std::filesystem::path(__FILE__).filename().string(), __LINE__)

    // 参数个数计数 最多9个
    #define ASSERT_ARG_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
    #define ASSERT_ARG_N(...) EXPAND_MACRO(ASSERT_ARG_COUNT(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

    // 安全的token拼接 先展开参数再拼接
    #define ASSERT_CAT_IMPL(a, b) a##b
    #define ASSERT_CAT(a, b) ASSERT_CAT_IMPL(a, b)

    // N=1->NO_MSG N>=2->WITH_MSG
    #define ASSERT_1(check) X_INTERNAL_ASSERT_NO_MSG(_, check)
    #define ASSERT_2(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_3(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_4(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_5(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_6(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_7(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_8(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)
    #define ASSERT_9(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(_, check, msg, ##__VA_ARGS__)

    #define CORE_ASSERT_1(check) INTERNAL_ASSERT_NO_MSG(CORE_, check)
    #define CORE_ASSERT_2(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_3(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_4(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_5(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_6(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_7(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_8(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)
    #define CORE_ASSERT_9(check, msg, ...) INTERNAL_ASSERT_WITH_MSG(CORE_, check, msg, ##__VA_ARGS__)

    // 展开后触发两轮宏替换 第一轮X_CAT拼出X_ASSERT_N 第二轮X_ASSERT_N展开为断言体 支持格式化字符串带变量
    #define ASSERT(...) EXPAND_MACRO(ASSERT_CAT(ASSERT_, ASSERT_ARG_N(__VA_ARGS__))(__VA_ARGS__))
    #define CORE_ASSERT(...) EXPAND_MACRO(ASSERT_CAT(CORE_ASSERT_, ASSERT_ARG_N(__VA_ARGS__))(__VA_ARGS__))
#else
    #define ASSERT(x, ...) ((void)(x))
    #define CORE_ASSERT(x, ...) ((void)(x))
#endif
