#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#endif

namespace mft {
namespace common {

/**
 * @brief Logging level enumeration
 */
enum class LogLevel {
    Verbose,
    Debug,
    Info,
    Warning,
    Error
};

/**
 * @brief Simple cross-platform logger
 * 
 * Uses spdlog if available, otherwise falls back to std::cerr.
 * Provides Serilog-style message templates with named parameters.
 */
class Logger {
public:
    static void set_level(LogLevel level);
    static LogLevel get_level();
    
    static void verbose(const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    
    // Template-based logging with format
    template<typename... Args>
    static void verbose_fmt(const char* fmt, Args&&... args) {
        if (should_log(LogLevel::Verbose)) {
            log_impl(LogLevel::Verbose, format_message(fmt, std::forward<Args>(args)...));
        }
    }
    
    template<typename... Args>
    static void debug_fmt(const char* fmt, Args&&... args) {
        if (should_log(LogLevel::Debug)) {
            log_impl(LogLevel::Debug, format_message(fmt, std::forward<Args>(args)...));
        }
    }
    
    template<typename... Args>
    static void info_fmt(const char* fmt, Args&&... args) {
        if (should_log(LogLevel::Info)) {
            log_impl(LogLevel::Info, format_message(fmt, std::forward<Args>(args)...));
        }
    }
    
    template<typename... Args>
    static void warning_fmt(const char* fmt, Args&&... args) {
        if (should_log(LogLevel::Warning)) {
            log_impl(LogLevel::Warning, format_message(fmt, std::forward<Args>(args)...));
        }
    }
    
    template<typename... Args>
    static void error_fmt(const char* fmt, Args&&... args) {
        if (should_log(LogLevel::Error)) {
            log_impl(LogLevel::Error, format_message(fmt, std::forward<Args>(args)...));
        }
    }

private:
    static LogLevel current_level_;
    static bool should_log(LogLevel level);
    static void log_impl(LogLevel level, const std::string& message);
    static std::string level_to_string(LogLevel level);
    
    // Simple variadic format helper
    template<typename T>
    static std::string to_string_helper(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    
    static std::string format_message(const char* fmt) {
        return fmt;
    }
    
    template<typename T, typename... Args>
    static std::string format_message(const char* fmt, T&& first, Args&&... rest) {
        std::string result;
        const char* p = fmt;
        
        while (*p) {
            if (*p == '{' && *(p + 1) != '\0') {
                // Find closing brace
                const char* end = p + 1;
                while (*end && *end != '}') ++end;
                if (*end == '}') {
                    // Replace placeholder with value
                    result += to_string_helper(first);
                    p = end + 1;
                    // Continue with remaining args
                    result += format_message(p, std::forward<Args>(rest)...);
                    return result;
                }
            }
            result += *p++;
        }
        return result;
    }
};

// Convenience macros
#define LOG_VERBOSE(msg) mft::common::Logger::verbose(msg)
#define LOG_DEBUG(msg) mft::common::Logger::debug(msg)
#define LOG_INFO(msg) mft::common::Logger::info(msg)
#define LOG_WARNING(msg) mft::common::Logger::warning(msg)
#define LOG_ERROR(msg) mft::common::Logger::error(msg)

} // namespace common
} // namespace mft
