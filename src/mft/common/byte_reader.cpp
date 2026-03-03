#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace mft {
namespace common {

bool ByteReader::is_little_endian() {
    uint16_t test = 1;
    return *reinterpret_cast<uint8_t*>(&test) == 1;
}

int16_t ByteReader::read_int16(const uint8_t* data, size_t offset) {
    int16_t value;
    std::memcpy(&value, data + offset, sizeof(value));
    // Data is always little-endian (NTFS), convert if needed
    if (!is_little_endian()) {
        value = static_cast<int16_t>((value >> 8) | (value << 8));
    }
    return value;
}

int32_t ByteReader::read_int32(const uint8_t* data, size_t offset) {
    int32_t value;
    std::memcpy(&value, data + offset, sizeof(value));
    if (!is_little_endian()) {
        value = ((value >> 24) & 0x000000FF) |
                ((value >> 8)  & 0x0000FF00) |
                ((value << 8)  & 0x00FF0000) |
                ((value << 24) & 0xFF000000);
    }
    return value;
}

int64_t ByteReader::read_int64(const uint8_t* data, size_t offset) {
    int64_t value;
    std::memcpy(&value, data + offset, sizeof(value));
    if (!is_little_endian()) {
        uint64_t uval = static_cast<uint64_t>(value);
        uval = ((uval >> 56) & 0x00000000000000FFULL) |
               ((uval >> 40) & 0x000000000000FF00ULL) |
               ((uval >> 24) & 0x0000000000FF0000ULL) |
               ((uval >> 8)  & 0x00000000FF000000ULL) |
               ((uval << 8)  & 0x000000FF00000000ULL) |
               ((uval << 24) & 0x0000FF0000000000ULL) |
               ((uval << 40) & 0x00FF000000000000ULL) |
               ((uval << 56) & 0xFF00000000000000ULL);
        value = static_cast<int64_t>(uval);
    }
    return value;
}

uint16_t ByteReader::read_uint16(const uint8_t* data, size_t offset) {
    return static_cast<uint16_t>(read_int16(data, offset));
}

uint32_t ByteReader::read_uint32(const uint8_t* data, size_t offset) {
    return static_cast<uint32_t>(read_int32(data, offset));
}

uint64_t ByteReader::read_uint64(const uint8_t* data, size_t offset) {
    return static_cast<uint64_t>(read_int64(data, offset));
}

uint8_t ByteReader::read_byte(const uint8_t* data, size_t offset) {
    return data[offset];
}

int16_t ByteReader::read_int16_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(int16_t) > data_len) {
        throw std::out_of_range("read_int16_safe: offset out of bounds");
    }
    return read_int16(data, offset);
}

int32_t ByteReader::read_int32_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(int32_t) > data_len) {
        throw std::out_of_range("read_int32_safe: offset out of bounds");
    }
    return read_int32(data, offset);
}

int64_t ByteReader::read_int64_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(int64_t) > data_len) {
        throw std::out_of_range("read_int64_safe: offset out of bounds");
    }
    return read_int64(data, offset);
}

uint16_t ByteReader::read_uint16_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(uint16_t) > data_len) {
        throw std::out_of_range("read_uint16_safe: offset out of bounds");
    }
    return read_uint16(data, offset);
}

uint32_t ByteReader::read_uint32_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(uint32_t) > data_len) {
        throw std::out_of_range("read_uint32_safe: offset out of bounds");
    }
    return read_uint32(data, offset);
}

uint64_t ByteReader::read_uint64_safe(const uint8_t* data, size_t data_len, size_t offset) {
    if (offset + sizeof(uint64_t) > data_len) {
        throw std::out_of_range("read_uint64_safe: offset out of bounds");
    }
    return read_uint64(data, offset);
}

std::string ByteReader::read_unicode_string(const uint8_t* data, size_t offset, size_t byte_length) {
    if (byte_length == 0) {
        return "";
    }
    
    // UTF-16LE to UTF-8 conversion
    std::string result;
    result.reserve(byte_length);
    
    for (size_t i = 0; i < byte_length; i += 2) {
        uint16_t code_unit = read_uint16(data, offset + i);
        
        if (code_unit == 0) {
            break;  // Null terminator
        }
        
        if (code_unit < 0x80) {
            // ASCII
            result.push_back(static_cast<char>(code_unit));
        } else if (code_unit < 0x800) {
            // 2-byte UTF-8
            result.push_back(static_cast<char>(0xC0 | (code_unit >> 6)));
            result.push_back(static_cast<char>(0x80 | (code_unit & 0x3F)));
        } else if (code_unit >= 0xD800 && code_unit <= 0xDBFF) {
            // Surrogate pair (high surrogate)
            if (i + 2 < byte_length) {
                uint16_t low = read_uint16(data, offset + i + 2);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    uint32_t code_point = 0x10000 + ((code_unit - 0xD800) << 10) + (low - 0xDC00);
                    result.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
                    result.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
                    result.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
                    result.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                    i += 2;  // Skip the low surrogate
                }
            }
        } else {
            // 3-byte UTF-8
            result.push_back(static_cast<char>(0xE0 | (code_unit >> 12)));
            result.push_back(static_cast<char>(0x80 | ((code_unit >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (code_unit & 0x3F)));
        }
    }
    
    return result;
}

std::string ByteReader::read_ascii_string(const uint8_t* data, size_t offset, size_t length) {
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        char c = static_cast<char>(data[offset + i]);
        if (c == '\0') {
            break;
        }
        result.push_back(c);
    }
    
    return result;
}

void ByteReader::block_copy(const uint8_t* src, size_t src_offset,
                           uint8_t* dst, size_t dst_offset, size_t count) {
    std::memcpy(dst + dst_offset, src + src_offset, count);
}

std::vector<uint8_t> ByteReader::extract_bytes(const uint8_t* data, size_t offset, size_t length) {
    return std::vector<uint8_t>(data + offset, data + offset + length);
}

std::string ByteReader::to_hex_string(const uint8_t* data, size_t length, 
                                      const std::string& separator) {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        if (i > 0) {
            oss << separator;
        }
        oss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) 
            << static_cast<int>(data[i]);
    }
    return oss.str();
}

} // namespace common
} // namespace mft
