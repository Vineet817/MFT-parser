#pragma once

#include "mft/mft_entry_info.hpp"
#include "mft/attributes/standard_info.hpp"
#include "mft/common/datetime_utils.hpp"
#include <cstdint>
#include <string>

namespace mft {
namespace attributes {

/**
 * @brief Name types for file names
 */
enum class NameTypes : uint8_t {
    Posix = 0x0,
    Windows = 0x1,
    Dos = 0x2,
    DosWindows = 0x3
};

/**
 * @brief Get string representation of name type
 */
std::string name_type_to_string(NameTypes type);

/**
 * @brief File information contained within FileName attribute
 * 
 * Contains parent reference, timestamps, sizes, flags, and name.
 */
class FileInfo {
public:
    /**
     * @brief Construct from raw bytes
     * @param raw_bytes Pointer to file info data
     * @param length Length of data
     */
    FileInfo(const uint8_t* raw_bytes, size_t length);
    
    // Parent reference
    const MftEntryInfo& parent_mft_record() const { return parent_mft_record_; }
    
    // Timestamps
    const common::NullableDateTime& created_on() const { return created_on_; }
    const common::NullableDateTime& content_modified_on() const { return content_modified_on_; }
    const common::NullableDateTime& record_modified_on() const { return record_modified_on_; }
    const common::NullableDateTime& last_accessed_on() const { return last_accessed_on_; }
    
    // Sizes
    uint64_t physical_size() const { return physical_size_; }
    uint64_t logical_size() const { return logical_size_; }
    
    // Flags and name
    StandardInfoFlag flags() const { return flags_; }
    int32_t reparse_value() const { return reparse_value_; }
    uint8_t name_length() const { return name_length_; }
    NameTypes name_type() const { return name_type_; }
    const std::string& file_name() const { return file_name_; }
    
    std::string to_string() const;

private:
    MftEntryInfo parent_mft_record_;
    
    common::NullableDateTime created_on_;
    common::NullableDateTime content_modified_on_;
    common::NullableDateTime record_modified_on_;
    common::NullableDateTime last_accessed_on_;
    
    uint64_t physical_size_;
    uint64_t logical_size_;
    
    StandardInfoFlag flags_;
    int32_t reparse_value_;
    uint8_t name_length_;
    NameTypes name_type_;
    std::string file_name_;
};

} // namespace attributes
} // namespace mft
