#pragma once

#include "x/core/base.h"

#include <fmt/format.h>
#include <glm/gtx/string_cast.hpp>
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

template <glm::length_t L, typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, T, Q>> : fmt::formatter<std::string> {
    auto format(const glm::vec<L, T, Q>& v, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(glm::to_string(v), ctx);
    }
};

template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
struct fmt::formatter<glm::mat<C, R, T, Q>> : fmt::formatter<std::string> {
    auto format(const glm::mat<C, R, T, Q>& m, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(glm::to_string(m), ctx);
    }
};

template <typename T, glm::qualifier Q>
struct fmt::formatter<glm::qua<T, Q>> : fmt::formatter<std::string> {
    auto format(const glm::qua<T, Q>& q, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(glm::to_string(q), ctx);
    }
};

// Core log macros
#define X_CORE_TRACE(...) ::Log::get_coreLogger()->trace(__VA_ARGS__)
#define X_CORE_INFO(...) ::Log::get_coreLogger()->info(__VA_ARGS__)
#define X_CORE_WARN(...) ::Log::get_coreLogger()->warn(__VA_ARGS__)
#define X_CORE_ERROR(...) ::Log::get_coreLogger()->error(__VA_ARGS__)
#define X_CORE_FATAL(...) ::Log::get_coreLogger()->critical(__VA_ARGS__)

// Client log macros
#define X_TRACE(...) ::Log::get_clientLogger()->trace(__VA_ARGS__)
#define X_INFO(...) ::Log::get_clientLogger()->info(__VA_ARGS__)
#define X_WARN(...) ::Log::get_clientLogger()->warn(__VA_ARGS__)
#define X_ERROR(...) ::Log::get_clientLogger()->error(__VA_ARGS__)
#define X_FATAL(...) ::Log::get_clientLogger()->critical(__VA_ARGS__)
