#include "engine/x_log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> XLog::s_coreLogger;
std::shared_ptr<spdlog::logger> XLog::s_clientLogger;

void XLog::Init() {
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

    s_coreLogger = std::make_shared<spdlog::logger>("ENGINE", sink);
    s_coreLogger->set_level(spdlog::level::trace);

    s_clientLogger = std::make_shared<spdlog::logger>("APP", sink);
    s_clientLogger->set_level(spdlog::level::trace);
}
