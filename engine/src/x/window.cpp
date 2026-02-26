//
// Created by rui ding on 2026/2/24.
//

#include "x/window.h"
#include "x/core.h" // 平台标识的宏定义在core里面

#if defined(X_PLATFORM_MAC)
#include "platform/mac_window.h"
#elif defined(X_PLATFORM_LINUX)
#include "platform/linux_window.h"
#else
#error "Unsupported platform!"
#endif

Window *Window::Create(const WindowProps &props)
{
#if defined(X_PLATFORM_MAC)
    return new MacWindow(props);
#elif defined(X_PLATFORM_LINUX)
    return new LinuxWindow(props);
#else
    return nullptr;  // 理论上不会走到这里
#endif
}