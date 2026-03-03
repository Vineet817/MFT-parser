#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <iomanip>
#include <sstream>

namespace mft {
namespace common {

/**
 * @brief DateTime utilities for Windows FILETIME conversion
 * 
 * Windows FILETIME is a 64-bit value representing the number of 
 * 100-nanosecond intervals since January 1, 1601 (UTC).
 */
class DateTimeUtils {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    
    // Windows epoch: January 1, 1601
    // Unix epoch: January 1, 1970
    // Difference in 100-nanosecond intervals
    static constexpr int64_t FILETIME_UNIX_DIFF = 116444736000000000LL;
    
    /**
     * @brief Convert Windows FILETIME to system_clock time_point
     * @param filetime 64-bit FILETIME value
     * @return Optional time_point (empty if invalid)
     */
    static std::optional<TimePoint> filetime_to_timepoint(int64_t filetime);
    
    /**
     * @brief Convert Unix timestamp to system_clock time_point
     * @param unix_seconds Seconds since Unix epoch
     * @return Time point
     */
    static TimePoint unix_to_timepoint(int64_t unix_seconds);
    
    /**
     * @brief Format time point to ISO 8601 string with microseconds
     * @param tp Time point to format
     * @return Formatted string like "2024-01-15 10:30:45.123456"
     */
    static std::string format_datetime(const TimePoint& tp);
    
    /**
     * @brief Format time point using custom format string
     * @param tp Time point to format
     * @param format strftime-compatible format string
     * @return Formatted string
     */
    static std::string format_datetime(const TimePoint& tp, const std::string& format);
    
    /**
     * @brief Get the default datetime format string
     * @return Format string "yyyy-MM-dd HH:mm:ss.fffffff"
     */
    static const std::string& default_format();
    
    /**
     * @brief Check if a FILETIME value is valid
     * @param filetime FILETIME value to check
     * @return true if valid, false otherwise
     */
    static bool is_valid_filetime(int64_t filetime);
};

/**
 * @brief Represents a nullable datetime similar to C#'s DateTimeOffset?
 */
class NullableDateTime {
public:
    NullableDateTime() : has_value_(false) {}
    
    explicit NullableDateTime(const DateTimeUtils::TimePoint& tp) 
        : value_(tp), has_value_(true) {}
    
    bool has_value() const { return has_value_; }
    
    const DateTimeUtils::TimePoint& value() const { 
        if (!has_value_) {
            throw std::runtime_error("NullableDateTime has no value");
        }
        return value_; 
    }
    
    std::string to_string() const {
        if (!has_value_) {
            return "";
        }
        return DateTimeUtils::format_datetime(value_);
    }
    
    std::string to_string(const std::string& format) const {
        if (!has_value_) {
            return "";
        }
        return DateTimeUtils::format_datetime(value_, format);
    }
    
    static NullableDateTime from_filetime(int64_t filetime) {
        auto tp = DateTimeUtils::filetime_to_timepoint(filetime);
        if (tp) {
            return NullableDateTime(*tp);
        }
        return NullableDateTime();
    }

private:
    DateTimeUtils::TimePoint value_;
    bool has_value_;
};

} // namespace common
} // namespace mft
