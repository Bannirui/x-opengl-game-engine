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

Input *Input::Create()
{
#if defined(X_PLATFORM_MAC)
    return new MacInput();
#elif defined(X_PLATFORM_LINUX)
    return new LinuxInput();
#else
    return nullptr;
#endif
}