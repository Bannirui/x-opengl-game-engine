#pragma once

#include <spdlog/spdlog.h>

#include "x/core/base.h"

class XLog
{
public:
    static void Init();

    static X::Ref<spdlog::logger> get_coreLogger() { return s_coreLogger; }
    static X::Ref<spdlog::logger> get_clientLogger() { return s_clientLogger; }

private:
    static X::Ref<spdlog::logger> s_coreLogger;
    static X::Ref<spdlog::logger> s_clientLogger;
};

// Core log macros
#define X_CORE_TRACE(...) ::XLog::get_coreLogger()->trace(__VA_ARGS__)
#define X_CORE_INFO(...) ::XLog::get_coreLogger()->info(__VA_ARGS__)
#define X_CORE_WARN(...) ::XLog::get_coreLogger()->warn(__VA_ARGS__)
#define X_CORE_ERROR(...) ::XLog::get_coreLogger()->error(__VA_ARGS__)
#define X_CORE_FATAL(...) ::XLog::get_coreLogger()->critical(__VA_ARGS__)

// Client log macros
#define X_TRACE(...) ::XLog::get_clientLogger()->trace(__VA_ARGS__)
#define X_INFO(...) ::XLog::get_clientLogger()->info(__VA_ARGS__)
#define X_WARN(...) ::XLog::get_clientLogger()->warn(__VA_ARGS__)
#define X_ERROR(...) ::XLog::get_clientLogger()->error(__VA_ARGS__)
#define X_FATAL(...) ::XLog::get_clientLogger()->critical(__VA_ARGS__)
