#include "mft/common/logger.hpp"

namespace mft {
namespace common {

LogLevel Logger::current_level_ = LogLevel::Info;

void Logger::set_level(LogLevel level) {
    current_level_ = level;
#ifdef USE_SPDLOG
    switch (level) {
        case LogLevel::Verbose:
            spdlog::set_level(spdlog::level::trace);
            break;
        case LogLevel::Debug:
            spdlog::set_level(spdlog::level::debug);
            break;
        case LogLevel::Info:
            spdlog::set_level(spdlog::level::info);
            break;
        case LogLevel::Warning:
            spdlog::set_level(spdlog::level::warn);
            break;
        case LogLevel::Error:
            spdlog::set_level(spdlog::level::err);
            break;
    }
#endif
}

LogLevel Logger::get_level() {
    return current_level_;
}

bool Logger::should_log(LogLevel level) {
    return static_cast<int>(level) >= static_cast<int>(current_level_);
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Verbose: return "VRB";
        case LogLevel::Debug:   return "DBG";
        case LogLevel::Info:    return "INF";
        case LogLevel::Warning: return "WRN";
        case LogLevel::Error:   return "ERR";
        default: return "???";
    }
}

void Logger::log_impl(LogLevel level, const std::string& message) {
#ifdef USE_SPDLOG
    switch (level) {
        case LogLevel::Verbose:
            spdlog::trace("{}", message);
            break;
        case LogLevel::Debug:
            spdlog::debug("{}", message);
            break;
        case LogLevel::Info:
            spdlog::info("{}", message);
            break;
        case LogLevel::Warning:
            spdlog::warn("{}", message);
            break;
        case LogLevel::Error:
            spdlog::error("{}", message);
            break;
    }
#else
    // Simple fallback logging to stderr
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    
    std::cerr << "[" << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << "] "
              << "[" << level_to_string(level) << "] "
              << message << std::endl;
#endif
}

void Logger::verbose(const std::string& message) {
    if (should_log(LogLevel::Verbose)) {
        log_impl(LogLevel::Verbose, message);
    }
}

void Logger::debug(const std::string& message) {
    if (should_log(LogLevel::Debug)) {
        log_impl(LogLevel::Debug, message);
    }
}

void Logger::info(const std::string& message) {
    if (should_log(LogLevel::Info)) {
        log_impl(LogLevel::Info, message);
    }
}

void Logger::warning(const std::string& message) {
    if (should_log(LogLevel::Warning)) {
        log_impl(LogLevel::Warning, message);
    }
}

void Logger::error(const std::string& message) {
    if (should_log(LogLevel::Error)) {
        log_impl(LogLevel::Error, message);
    }
}

} // namespace common
} // namespace mft
