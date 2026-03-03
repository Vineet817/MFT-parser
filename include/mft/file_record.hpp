#pragma once

#include "mft/mft_entry_info.hpp"
#include "mft/fixup_data.hpp"
#include "mft/attributes/attribute.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace mft {

// Forward declarations
namespace attributes {
    class StandardInfo;
    class FileName;
    class Data;
}
namespace other {
    class IndexEntryI30;
}

/**
 * @brief Entry flags for FILE records
 */
enum class EntryFlag : int16_t {
    IsFree = 0x0,
    InUse = 0x1,
    IsDirectory = 0x2,
    IsMetaDataRecord = 0x4,
    IsIndexView = 0x8
};

// Bitwise operators for EntryFlag
inline EntryFlag operator|(EntryFlag a, EntryFlag b) {
    return static_cast<EntryFlag>(static_cast<int16_t>(a) | static_cast<int16_t>(b));
}

inline EntryFlag operator&(EntryFlag a, EntryFlag b) {
    return static_cast<EntryFlag>(static_cast<int16_t>(a) & static_cast<int16_t>(b));
}

inline bool has_flag(EntryFlag flags, EntryFlag flag) {
    return (static_cast<int16_t>(flags) & static_cast<int16_t>(flag)) != 0;
}

std::string entry_flag_to_string(EntryFlag flags);

/**
 * @brief Represents a single FILE record in the MFT
 * 
 * A FILE record is typically 1024 bytes and contains one or more attributes
 * that describe a file or directory.
 */
class FileRecord {
public:
    static constexpr int32_t BAAD_SIG = 0x44414142;  // "BAAD"
    static constexpr int32_t FILE_SIG = 0x454C4946;  // "FILE"
    
    /**
     * @brief Construct from raw bytes
     * @param raw_bytes Pointer to raw record data
     * @param length Length of data
     * @param offset Absolute offset in MFT
     * @param recover_from_slack Whether to attempt slack space recovery
     */
    FileRecord(const uint8_t* raw_bytes, size_t length, int offset, bool recover_from_slack);
    
    // Basic properties
    int offset() const { return offset_; }
    uint32_t entry_number() const { return entry_number_; }
    uint16_t sequence_number() const { return sequence_number_; }
    EntryFlag entry_flags() const { return entry_flags_; }
    int64_t log_sequence_number() const { return log_sequence_number_; }
    
    // Record info
    int16_t first_attribute_offset() const { return first_attribute_offset_; }
    int32_t actual_record_size() const { return actual_record_size_; }
    int32_t allocated_record_size() const { return allocated_record_size_; }
    const MftEntryInfo& mft_record_to_base_record() const { return mft_record_to_base_record_; }
    int16_t first_available_attribute_id() const { return first_available_attribute_id_; }
    int16_t reference_count() const { return reference_count_; }
    
    // Fixup info
    int16_t fixup_offset() const { return fixup_offset_; }
    int16_t fixup_entry_count() const { return fixup_entry_count_; }
    const FixupData& fixup_data() const { return fixup_data_; }
    bool fixup_ok() const { return fixup_ok_; }
    
    // Status flags
    bool is_bad() const { return is_bad_; }
    bool is_uninitialized() const { return is_uninitialized_; }
    
    // Attributes
    const std::vector<std::unique_ptr<attributes::Attribute>>& attributes() const { 
        return attributes_; 
    }
    std::vector<std::unique_ptr<attributes::Attribute>>& attributes() { 
        return attributes_; 
    }
    
    // Helper methods
    bool is_directory() const;
    bool is_deleted() const;
    std::string get_key(bool as_decimal = false) const;
    
    std::string to_string() const;
    
    /**
     * @brief Recover index entries from slack space
     */
    static std::vector<std::unique_ptr<other::IndexEntryI30>> get_slack_file_entries(
        const uint8_t* slack_space, size_t length, 
        int page_number, int start_offset, uint32_t entry_number);

private:
    int offset_;
    uint32_t entry_number_;
    uint16_t sequence_number_;
    EntryFlag entry_flags_;
    int64_t log_sequence_number_;
    
    int16_t first_attribute_offset_;
    int32_t actual_record_size_;
    int32_t allocated_record_size_;
    MftEntryInfo mft_record_to_base_record_;
    int16_t first_available_attribute_id_;
    int16_t reference_count_;
    
    int16_t fixup_offset_;
    int16_t fixup_entry_count_;
    FixupData fixup_data_;
    bool fixup_ok_;
    
    bool is_bad_;
    bool is_uninitialized_;
    
    std::vector<std::unique_ptr<attributes::Attribute>> attributes_;
    
    // Parse attribute from raw bytes and add to attributes vector
    void parse_attribute(const uint8_t* raw_bytes, size_t length, 
                        attributes::AttributeType type, int abs_offset);
};

/**
 * @brief Helper struct for Unicode string hits in slack space
 */
struct HitInfo {
    int offset;
    std::string hit;
    
    HitInfo(int off, std::string h) : offset(off), hit(std::move(h)) {}
};

} // namespace mft
