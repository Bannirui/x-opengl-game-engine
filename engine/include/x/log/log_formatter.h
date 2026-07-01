#pragma once

#include "x/events/event.h"
#include "x/log/log.h"

#include <fmt/format.h>
#include <glm/gtx/string_cast.hpp>

inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.ToString();
}

// for Event
template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>> : fmt::formatter<std::string> {
    auto format(const Event& e, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(e.ToString(), ctx);
    }
};

// for glm
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
