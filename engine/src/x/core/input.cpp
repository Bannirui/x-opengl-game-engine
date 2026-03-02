//
// Created by rui ding on 2026/2/26.
//

#include "x/core/input.h"

#include "platform/mac/mac_input.h"
#include "x/core/base.h"

#if defined(X_PLATFORM_MAC)
#include "platform/mac/mac_input.h"
#elif defined(X_PLATFORM_LINUX)
#include "platform/linux/linux_input.h"
#else
#error "Unsupported platform!"
#endif

X::Scope<Input> Input::s_instance = Input::Create();

X::Scope<Input> Input::Create()
{
#if defined(X_PLATFORM_MAC)
    return X::CreateScope<MacInput>();
#elif defined(X_PLATFORM_LINUX)
    return X::CreateScope<LinuxInput>();
#endif
}