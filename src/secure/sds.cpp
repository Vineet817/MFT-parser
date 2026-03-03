#include "secure/sds.hpp"
#include "mft/common/logger.hpp"
#include "mft/common/byte_reader.hpp"
#include <fstream>

namespace secure {

Sds::Sds(std::istream& file_stream) {
    using mft::common::ByteReader;
    using mft::common::Logger;

    file_stream.seekg(0, std::ios::end);
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    const size_t BUFFER_SIZE = 256 * 1024; // 256KB chunks (typical SDS chunk size)
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    
    size_t current_offset = 0;
    Logger::info_fmt("Parsing $SDS (size: {} bytes)", file_size);

    while (current_offset < file_size && file_stream.good()) {
        file_stream.seekg(current_offset);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);
        size_t bytes_read = file_stream.gcount();

        if (bytes_read < 20) break;

        size_t buffer_pos = 0;
        
        while (buffer_pos + 20 <= bytes_read) {
            uint32_t hash = ByteReader::read_uint32(buffer.data(), buffer_pos);
            uint32_t security_id = ByteReader::read_uint32(buffer.data(), buffer_pos + 4);
            uint64_t offset = ByteReader::read_uint64(buffer.data(), buffer_pos + 8);
            uint32_t length = ByteReader::read_uint32(buffer.data(), buffer_pos + 16);
            
            // Check for valid entry
            if (length < 20 || length > 1024 * 64) {
                // Invalid length, likely padding or end of chunk
                // In SDS, chunks are often padded with nulls at the end
                // Align to 16 bytes and continue? 
                // Usually once we hit zeros, the rest of the 256KB chunk is zeros.
                break; 
            }
            
            // Validate offset
            if (offset != current_offset + buffer_pos) {
                 // Mismatch offset - might be traversing garbage
            }

            // TODO: Store or process this descriptor
            // For now, valid entry found
            // Logger::debug_fmt("Found Security ID: {}, Hash: 0x{:X}", security_id, hash);
            
            // Align to 16 bytes
            size_t aligned_len = (length + 15) & ~15;
            
            buffer_pos += aligned_len;
        }

        // Move to next 256KB chunk
        current_offset += BUFFER_SIZE;
    }
    
    Logger::info("SDS parsing completed");
}

std::unique_ptr<Sds> SdsFile::load(const std::string& sds_path) {
    std::ifstream file(sds_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("'" + sds_path + "' not found");
    }
    return std::make_unique<Sds>(file);
}

} // namespace secure
