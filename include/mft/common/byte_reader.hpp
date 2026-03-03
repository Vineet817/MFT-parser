#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <stdexcept>

namespace mft {
namespace common {

/**
 * @brief Cross-platform binary reader for little-endian data
 * 
 * Provides safe reading of binary data with bounds checking and
 * automatic little-endian to host byte order conversion.
 */
class ByteReader {
public:
    // Read signed integers
    static int16_t read_int16(const uint8_t* data, size_t offset);
    static int32_t read_int32(const uint8_t* data, size_t offset);
    static int64_t read_int64(const uint8_t* data, size_t offset);
    
    // Read unsigned integers
    static uint16_t read_uint16(const uint8_t* data, size_t offset);
    static uint32_t read_uint32(const uint8_t* data, size_t offset);
    static uint64_t read_uint64(const uint8_t* data, size_t offset);
    
    // Read single byte
    static uint8_t read_byte(const uint8_t* data, size_t offset);
    
    // Read with bounds checking
    static int16_t read_int16_safe(const uint8_t* data, size_t data_len, size_t offset);
    static int32_t read_int32_safe(const uint8_t* data, size_t data_len, size_t offset);
    static int64_t read_int64_safe(const uint8_t* data, size_t data_len, size_t offset);
    static uint16_t read_uint16_safe(const uint8_t* data, size_t data_len, size_t offset);
    static uint32_t read_uint32_safe(const uint8_t* data, size_t data_len, size_t offset);
    static uint64_t read_uint64_safe(const uint8_t* data, size_t data_len, size_t offset);
    
    /**
     * @brief Read UTF-16LE encoded string (Windows Unicode)
     * @param data Source buffer
     * @param offset Byte offset into buffer
     * @param byte_length Length in bytes (not characters)
     * @return UTF-8 encoded string
     */
    static std::string read_unicode_string(const uint8_t* data, size_t offset, size_t byte_length);
    
    /**
     * @brief Read ASCII/ANSI encoded string
     * @param data Source buffer
     * @param offset Byte offset into buffer
     * @param length Length in bytes
     * @return String
     */
    static std::string read_ascii_string(const uint8_t* data, size_t offset, size_t length);
    
    /**
     * @brief Copy bytes from source to destination (like Buffer.BlockCopy)
     * @param src Source buffer
     * @param src_offset Offset in source
     * @param dst Destination buffer
     * @param dst_offset Offset in destination
     * @param count Number of bytes to copy
     */
    static void block_copy(const uint8_t* src, size_t src_offset,
                          uint8_t* dst, size_t dst_offset, size_t count);
    
    /**
     * @brief Extract a sub-array from a buffer
     * @param data Source buffer
     * @param offset Starting offset
     * @param length Number of bytes
     * @return New vector containing the bytes
     */
    static std::vector<uint8_t> extract_bytes(const uint8_t* data, size_t offset, size_t length);
    
    /**
     * @brief Convert bytes to hex string representation
     * @param data Source buffer
     * @param length Number of bytes
     * @param separator Separator between hex values (default "-")
     * @return Hex string like "4F-49-4C-45"
     */
    static std::string to_hex_string(const uint8_t* data, size_t length, 
                                     const std::string& separator = "-");

private:
    // Helper to check if system is little-endian
    static bool is_little_endian();
};

} // namespace common
} // namespace mft
