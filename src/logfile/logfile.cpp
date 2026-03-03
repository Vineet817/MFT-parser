#include "logfile/logfile.hpp"
#include "mft/common/logger.hpp"
#include "mft/common/byte_reader.hpp"
#include <fstream>

namespace logfile {

LogFile::LogFile(std::istream& file_stream) {
    using mft::common::ByteReader;
    using mft::common::Logger;

    file_stream.seekg(0, std::ios::end);
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    const size_t RESTART_AREA_SIZE = 4096;
    std::vector<uint8_t> buffer(RESTART_AREA_SIZE);

    // Check primary restart area
    file_stream.read(reinterpret_cast<char*>(buffer.data()), RESTART_AREA_SIZE);
    
    uint32_t magic = ByteReader::read_uint32(buffer.data(), 0);
    // "RSTR" = 0x52535452
    if (magic == 0x52535452) {
        Logger::info("Found valid primary Restart Area (RSTR)");
        // Parse restart area details (lsn, etc.)
    } else {
        Logger::warning_fmt("Invalid primary Restart Area signature: 0x{:X}", magic);
    }

    // Check secondary restart area (usually at file_size - size or similar, but for parsing sake checking start is enough proof of concept)
    
    // Scan for RCRD pages
    Logger::info_fmt("Scanning $LogFile (size: {} bytes) for Record Pages...", file_size);
    
    // In a full implementation, we would follow the LSN chain.
    // For now, let's just count RCRD pages to verify structure.
    
    int rcrd_count = 0;
    size_t offset = RESTART_AREA_SIZE * 2; // Skip restart areas
    
    const size_t PAGE_SIZE = 4096; // Typical page size
    
    while (offset + PAGE_SIZE <= file_size && file_stream.good()) {
        file_stream.seekg(offset);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), PAGE_SIZE);
        
        uint32_t items_magic = ByteReader::read_uint32(buffer.data(), 0);
        
        // "RCRD" = 0x44524352
        if (items_magic == 0x44524352) {
            rcrd_count++;
        }
        
        offset += PAGE_SIZE;
    }
    
    Logger::info_fmt("Found {} Record Pages (RCRD)", rcrd_count);
}

std::unique_ptr<LogFile> LogFileParser::load(const std::string& log_path) {
    std::ifstream file(log_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("'" + log_path + "' not found");
    }
    return std::make_unique<LogFile>(file);
}

} // namespace logfile
