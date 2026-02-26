//
// Created by rui ding on 2026/2/26.
//

#include "x/input.h"
#include "x/core.h"  // 平台标识的宏定义在core里面

#if defined(X_PLATFORM_MAC)
#include "platform/mac/mac_input.h"
#elif defined(X_PLATFORM_LINUX)
#include "platform/linux/linux_input.h"
#else
#error "Unsupported platform!"
#endif

Input *Input::s_instance = nullptr;

Input *Input::Create()
{
#if defined(X_PLATFORM_MAC)
    s_instance = new MacInput();
    return s_instance;
#elif defined(X_PLATFORM_LINUX)
    s_instance = new LinuxInput();
    return s_instance;
#else
    return nullptr;
#endif
}