//
// Created by rui ding on 2026/2/26.
//

#include "x/core/input.h"

#include "platform/mac/mac_input.h"
#include "x/core.h"

#if defined(X_PLATFORM_MAC)
#include "platform/mac/mac_input.h"
#elif defined(X_PLATFORM_LINUX)
#include "platform/linux/linux_input.h"
#else
#error "Unsupported platform!"
#endif

X::Scope<Input> Input::s_instance;

void Input::Create()
{
#if defined(X_PLATFORM_MAC)
    s_instance = X::CreateScope<MacInput>();
#elif defined(X_PLATFORM_LINUX)
    s_instance = X::CreateScope<LinuxInput>();
#endif
}