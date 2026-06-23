#pragma once

#include "x/core/log.h"
#include "x/events/event.h"

inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.ToString();
}

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<Event, T>::value, char>> : fmt::formatter<std::string> {
    auto format(const Event& e, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(e.ToString(), ctx);
    }
};
