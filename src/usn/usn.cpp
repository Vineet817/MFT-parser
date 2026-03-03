#include "usn/usn.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <fstream>
#include <sstream>

namespace usn {

UsnRecord::UsnRecord(const uint8_t* raw_bytes, size_t length) {
    using mft::common::ByteReader;
    
    record_length_ = ByteReader::read_uint32(raw_bytes, 0);
    major_version_ = ByteReader::read_uint16(raw_bytes, 4);
    minor_version_ = ByteReader::read_uint16(raw_bytes, 6);
    
    if (major_version_ == 2) {
        file_reference_number_ = ByteReader::read_uint64(raw_bytes, 8);
        parent_file_reference_number_ = ByteReader::read_uint64(raw_bytes, 16);
        usn_ = ByteReader::read_uint64(raw_bytes, 24);
        
        int64_t ts_raw = ByteReader::read_int64(raw_bytes, 32);
        try {
            timestamp_ = mft::common::NullableDateTime::from_filetime(ts_raw);
        } catch(...) {}
        
        reason_ = ByteReader::read_uint32(raw_bytes, 40);
        source_info_ = ByteReader::read_uint32(raw_bytes, 44);
        security_id_ = ByteReader::read_uint32(raw_bytes, 48);
        file_attributes_ = ByteReader::read_uint32(raw_bytes, 52);
        
        uint16_t name_len = ByteReader::read_uint16(raw_bytes, 56);
        uint16_t name_offset = ByteReader::read_uint16(raw_bytes, 58);
        
        if (name_len > 0 && name_offset + name_len <= length) {
            filename_ = ByteReader::read_unicode_string(raw_bytes, name_offset, name_len);
        }
    }
}

std::string UsnRecord::to_string() const {
    std::ostringstream oss;
    oss << "USN: 0x" << std::hex << usn_ 
        << " File: " << filename_
        << " Reason: 0x" << reason_
        << " Time: " << timestamp_.to_string();
    return oss.str();
}

UsnJournal::UsnJournal(std::istream& file_stream) {
    using mft::common::ByteReader;
    using mft::common::Logger;

    file_stream.seekg(0, std::ios::end);
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    const size_t BUFFER_SIZE = 1024 * 1024; // 1MB chunks
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    
    // Keep track of where we are in the file
    size_t current_file_offset = 0;

    Logger::info_fmt("Parsing USN Journal (size: {} bytes)", file_size);

    while (current_file_offset < file_size && file_stream.good()) {
        file_stream.seekg(current_file_offset);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);
        size_t bytes_read = file_stream.gcount();

        if (bytes_read < 8) break; // End of file or too small

        size_t buffer_pos = 0;
        bool buffer_consumed = true;

        while (buffer_pos + 8 <= bytes_read) {
            // Check for zero record length (padding or sparse area)
            uint32_t record_len = ByteReader::read_uint32(buffer.data(), buffer_pos);

            if (record_len == 0) {
                // Sparse area or padding. Skip 8 bytes (alignment) or scan forward?
                // Simple optimization: Skip 4 bytes and continue
                buffer_pos += 4;
                // Align to 8 bytes boundary if needed, but USN is usually 8-byte aligned
                continue;
            }

            // Sanity check: reasonable size?
            if (record_len > 1024 * 64 || record_len < 8) {
                // Likely garbage or we lost sync. 
                // For sparse files, we might be in a region of partial zeros.
                // Just skip 4 bytes and retry.
                buffer_pos += 4;
                continue;
            }

            // Check if record fits in buffer
            if (buffer_pos + record_len > bytes_read) {
                // Record straddles buffer boundary
                // Stop processing this buffer, next iteration will pick up from current_file_offset + buffer_pos
                buffer_consumed = false;
                break;
            }

            // Parse record
            try {
                records_.emplace_back(buffer.data() + buffer_pos, record_len);
                // Valid record found
            } catch (...) {
                // Parse error, skip
            }

            buffer_pos += record_len;
        }

        // Advance file offset
        if (buffer_consumed) {
            current_file_offset += bytes_read;
        } else {
            // We stopped early because a record was cut off
            current_file_offset += buffer_pos;
        }
    }
    
    Logger::info_fmt("Parsed {} USN records", records_.size());
}

std::unique_ptr<UsnJournal> UsnFile::load(const std::string& usn_path) {
    std::ifstream file(usn_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("'" + usn_path + "' not found");
    }
    return std::make_unique<UsnJournal>(file);
}

} // namespace usn
