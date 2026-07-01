#pragma once

#include "x/core/base.h"

#include <spdlog/spdlog.h>

class Log {
public:
    static void Init();

    static Ref<spdlog::logger>& get_coreLogger() {
        return s_coreLogger;
    }

    static Ref<spdlog::logger>& get_clientLogger() {
        return s_clientLogger;
    }

private:
    static Ref<spdlog::logger> s_coreLogger;
    static Ref<spdlog::logger> s_clientLogger;
};

// Core log macros
#define CORE_TRACE(...) ::Log::get_coreLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...) ::Log::get_coreLogger()->info(__VA_ARGS__)
#define CORE_WARN(...) ::Log::get_coreLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...) ::Log::get_coreLogger()->error(__VA_ARGS__)
#define CORE_FATAL(...) ::Log::get_coreLogger()->critical(__VA_ARGS__)

// Client log macros
#define TRACE(...) ::Log::get_clientLogger()->trace(__VA_ARGS__)
#define INFO(...) ::Log::get_clientLogger()->info(__VA_ARGS__)
#define WARN(...) ::Log::get_clientLogger()->warn(__VA_ARGS__)
#define ERROR(...) ::Log::get_clientLogger()->error(__VA_ARGS__)
#define FATAL(...) ::Log::get_clientLogger()->critical(__VA_ARGS__)
