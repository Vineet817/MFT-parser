#include "mft/common/datetime_utils.hpp"

namespace mft {
namespace common {

std::optional<DateTimeUtils::TimePoint> DateTimeUtils::filetime_to_timepoint(int64_t filetime) {
    if (!is_valid_filetime(filetime)) {
        return std::nullopt;
    }
    
    // Convert FILETIME to Unix timestamp in 100-nanosecond intervals
    int64_t unix_100ns = filetime - FILETIME_UNIX_DIFF;
    
    // Convert to microseconds (100ns * 10 = 1000ns = 1us)
    int64_t unix_us = unix_100ns / 10;
    
    // Create time_point from microseconds
    auto duration = std::chrono::microseconds(unix_us);
    return TimePoint(std::chrono::duration_cast<TimePoint::duration>(duration));
}

DateTimeUtils::TimePoint DateTimeUtils::unix_to_timepoint(int64_t unix_seconds) {
    return TimePoint(std::chrono::seconds(unix_seconds));
}

std::string DateTimeUtils::format_datetime(const TimePoint& tp) {
    return format_datetime(tp, "%Y-%m-%d %H:%M:%S");
}

std::string DateTimeUtils::format_datetime(const TimePoint& tp, const std::string& format) {
    auto time_t_val = std::chrono::system_clock::to_time_t(tp);
    auto* tm = std::gmtime(&time_t_val);
    
    if (!tm) {
        return "<invalid time>";
    }
    
    std::ostringstream oss;
    oss << std::put_time(tm, format.c_str());
    
    // Add sub-second precision if format doesn't already include it
    // Calculate microseconds part
    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration - seconds);
    
    // Append .ffffff if the format ends with seconds
    if (format.find("%S") != std::string::npos || format.find("%s") != std::string::npos) {
        oss << "." << std::setfill('0') << std::setw(7) << (micros.count() * 10);
    }
    
    return oss.str();
}

const std::string& DateTimeUtils::default_format() {
    static const std::string fmt = "%Y-%m-%d %H:%M:%S";
    return fmt;
}

bool DateTimeUtils::is_valid_filetime(int64_t filetime) {
    // FILETIME must be positive and not too far in the future
    // Min: 0 (Jan 1, 1601)
    // Max: Reasonable limit (year 3000 or so)
    if (filetime <= 0) {
        return false;
    }
    
    // Check if it would result in a reasonable Unix timestamp
    int64_t unix_100ns = filetime - FILETIME_UNIX_DIFF;
    
    // Must be after Unix epoch (1970) - allow some pre-1970 dates
    // and before year 3000
    // Max reasonable value: around 3000 AD
    constexpr int64_t MAX_FILETIME = 157766016000000000LL; // ~3000 AD
    
    return filetime < MAX_FILETIME;
}

} // namespace common
} // namespace mft
