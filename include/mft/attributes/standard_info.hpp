#pragma once

#include "mft/attributes/attribute.hpp"
#include "mft/common/datetime_utils.hpp"
#include <cstdint>

namespace mft {
namespace attributes {

/**
 * @brief Standard Information attribute flags
 */
enum class StandardInfoFlag : int32_t {
    None = 0x00,
    ReadOnly = 0x01,
    Hidden = 0x02,
    System = 0x04,
    VolumeLabel = 0x08,
    Directory = 0x010,
    Archive = 0x020,
    Device = 0x040,
    Normal = 0x080,
    Temporary = 0x0100,
    SparseFile = 0x0200,
    ReparsePoint = 0x0400,
    Compressed = 0x0800,
    Offline = 0x01000,
    NotContentIndexed = 0x02000,
    Encrypted = 0x04000,
    IntegrityStream = 0x08000,
    Virtual = 0x010000,
    NoScrubData = 0x020000,
    RecallOnOpen = 0x040000,
    RecallOnDataAccess = 0x400000,
    Pinned = 0x80000,
    UnPinned = 0x100000,
    IsDirectory = 0x10000000,
    IsIndexView = 0x20000000
};

// Bitwise operators for StandardInfoFlag
inline StandardInfoFlag operator|(StandardInfoFlag a, StandardInfoFlag b) {
    return static_cast<StandardInfoFlag>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
}

inline StandardInfoFlag operator&(StandardInfoFlag a, StandardInfoFlag b) {
    return static_cast<StandardInfoFlag>(static_cast<int32_t>(a) & static_cast<int32_t>(b));
}

inline bool has_flag(StandardInfoFlag flags, StandardInfoFlag flag) {
    return (static_cast<int32_t>(flags) & static_cast<int32_t>(flag)) != 0;
}

/**
 * @brief Get string representation of flags
 */
std::string standard_info_flag_to_string(StandardInfoFlag flags);

/**
 * @brief Secondary flags for Standard Information
 */
enum class StandardInfoFlag2 : int32_t {
    None = 0x00,
    IsCaseSensitive = 0x01
};

/**
 * @brief Standard Information attribute ($STANDARD_INFORMATION)
 * 
 * Contains timestamps, flags, owner/security information.
 * Present in every file record.
 */
class StandardInfo : public Attribute {
public:
    /**
     * @brief Construct from raw bytes
     */
    StandardInfo(const uint8_t* raw_bytes, size_t length);
    
    // Timestamps
    const common::NullableDateTime& created_on() const { return created_on_; }
    const common::NullableDateTime& content_modified_on() const { return content_modified_on_; }
    const common::NullableDateTime& record_modified_on() const { return record_modified_on_; }
    const common::NullableDateTime& last_accessed_on() const { return last_accessed_on_; }
    
    // Flags
    StandardInfoFlag flags() const { return flags_; }
    StandardInfoFlag2 flags2() const { return flags2_; }
    
    // Other properties
    int32_t max_version() const { return max_version_; }
    int32_t class_id() const { return class_id_; }
    int32_t owner_id() const { return owner_id_; }
    int32_t security_id() const { return security_id_; }
    int32_t quota_charged() const { return quota_charged_; }
    int64_t update_sequence_number() const { return update_sequence_number_; }
    
    std::string to_string() const override;

private:
    common::NullableDateTime created_on_;
    common::NullableDateTime content_modified_on_;
    common::NullableDateTime record_modified_on_;
    common::NullableDateTime last_accessed_on_;
    
    StandardInfoFlag flags_;
    StandardInfoFlag2 flags2_;
    
    int32_t max_version_;
    int32_t class_id_;
    int32_t owner_id_;
    int32_t security_id_;
    int32_t quota_charged_;
    int64_t update_sequence_number_;
};

} // namespace attributes
} // namespace mft
