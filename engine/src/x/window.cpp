//
// Created by rui ding on 2026/2/24.
//

#include "x/window.h"
#include "x/core/core.h"

#if defined(X_PLATFORM_MAC)
#include "platform/mac/mac_window.h"
#elif defined(X_PLATFORM_LINUX)
#include "platform/linux/linux_window.h"
#else
#error "Unsupported platform!"
#endif

X::Scope<Window> Window::Create(const WindowProps &props)
{
#if defined(X_PLATFORM_MAC)
    return X::CreateScope<MacWindow>(props);
#elif defined(X_PLATFORM_LINUX)
    return X::CreateScope<LinuxWindow>(props);
#else
    return nullptr;  // 理论上不会走到这里
#endif
}