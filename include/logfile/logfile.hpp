#pragma once

#include <vector>
#include <string>
#include <memory>
#include <istream>

namespace logfile {

/**
 * @brief $LogFile Parser
 */
class LogFile {
public:
    explicit LogFile(std::istream& file_stream);
};

class LogFileParser {
public:
    static std::unique_ptr<LogFile> load(const std::string& log_path);
};

} // namespace logfile
