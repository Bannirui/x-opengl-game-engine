//
// Created by dingrui on 3/4/26.
//

#pragma once

// ================================
// Platform detection
// ================================
#if defined(_WIN32)
#    ifdef _WIN64
#        define X_PLATFORM_WINDOWS
#    else
#        error "x86 Builds are not supported"
#    endif
#elif defined(__APPLE__) || defined(__MACH__)
#    include <TargetConditionals.h>
#    if TARGET_IPHONE_SIMULATOR == 1
#        error "IOS simulator is not supported"
#    elif TARGET_OS_IPHONE == 1
#        define X_PLATFORM_IOS
#        error "IOS is not supported"
#    elif TARGET_OS_MAC == 1
#        define X_PLATFORM_MAC
#    else
#        error "Unkonown Apple platform"
#    endif
#elif defined(__ANDROID__)
#    define X_PLATFORM_ANDROID
#    error "Android is not supported"
#elif defined(__linux__)
#    define X_PLATFORM_LINUX
#else
#    error "Unknown platform!"
#endif
