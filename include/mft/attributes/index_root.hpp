#pragma once

#include "mft/attributes/attribute.hpp"
#include "mft/mft_entry_info.hpp"
#include "mft/attributes/file_info.hpp"
#include <vector>
#include <memory>

namespace mft {
namespace other {

/**
 * @brief Index entry within IndexRoot
 */
class IndexEntry {
public:
    IndexEntry(const uint8_t* raw_bytes, size_t length);
    
    const common::NullableDateTime& created_on() const { return created_on_; }
    const common::NullableDateTime& content_modified_on() const { return content_modified_on_; }
    const common::NullableDateTime& record_modified_on() const { return record_modified_on_; }
    const common::NullableDateTime& last_accessed_on() const { return last_accessed_on_; }
    
    uint64_t physical_size() const { return physical_size_; }
    uint64_t logical_size() const { return logical_size_; }
    attributes::StandardInfoFlag flags() const { return flags_; }
    int32_t reparse_value() const { return reparse_value_; }
    uint8_t name_length() const { return name_length_; }
    attributes::NameTypes name_type() const { return name_type_; }
    const std::string& file_name() const { return file_name_; }
    
    const MftEntryInfo& parent_mft_record() const { return parent_mft_record_; }
    
    std::string to_string() const;

private:
    MftEntryInfo parent_mft_record_;
    common::NullableDateTime created_on_;
    common::NullableDateTime content_modified_on_;
    common::NullableDateTime record_modified_on_;
    common::NullableDateTime last_accessed_on_;
    uint64_t physical_size_;
    uint64_t logical_size_;
    attributes::StandardInfoFlag flags_;
    int32_t reparse_value_;
    uint8_t name_length_;
    attributes::NameTypes name_type_;
    std::string file_name_;
};

} // namespace other

namespace attributes {

/**
 * @brief Collation types for index
 */
enum class CollationType : int32_t {
    Binary = 0x000000,
    Filename = 0x000001,
    Unicode = 0x000002,
    NtOfsUlong = 0x000010,
    NtOfsSid = 0x000011,
    NtOfsSecurityHash = 0x000012,
    NtOfsUlongs = 0x000013
};

/**
 * @brief Index flags
 */
enum class IndexFlag : int32_t {
    None = 0x000,
    HasSubNode = 0x001,
    IsLast = 0x002
};

inline IndexFlag operator|(IndexFlag a, IndexFlag b) {
    return static_cast<IndexFlag>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
}

inline IndexFlag operator&(IndexFlag a, IndexFlag b) {
    return static_cast<IndexFlag>(static_cast<int32_t>(a) & static_cast<int32_t>(b));
}

inline bool has_flag(IndexFlag flags, IndexFlag flag) {
    return (static_cast<int32_t>(flags) & static_cast<int32_t>(flag)) != 0;
}

/**
 * @brief IndexRoot attribute ($INDEX_ROOT)
 * 
 * Contains the root of an index (typically for directories).
 */
class IndexRoot : public Attribute {
public:
    IndexRoot(const uint8_t* raw_bytes, size_t length);
    
    AttributeType indexed_attribute_type() const { return indexed_attribute_type_; }
    CollationType collation_type() const { return collation_type_; }
    int32_t entry_size() const { return entry_size_; }
    int32_t number_cluster_blocks() const { return number_cluster_blocks_; }
    int32_t offset_to_first_index_entry() const { return offset_to_first_index_entry_; }
    int32_t total_size_of_index_entries() const { return total_size_of_index_entries_; }
    int32_t allocated_size_of_entries() const { return allocated_size_of_entries_; }
    IndexFlag flags() const { return flags_; }
    
    const MftEntryInfo& mft_record() const { return mft_record_; }
    const std::vector<other::IndexEntry>& index_entries() const { return index_entries_; }
    
    std::string to_string() const override;

private:
    AttributeType indexed_attribute_type_;
    CollationType collation_type_;
    int32_t entry_size_;
    int32_t number_cluster_blocks_;
    int32_t offset_to_first_index_entry_;
    int32_t total_size_of_index_entries_;
    int32_t allocated_size_of_entries_;
    IndexFlag flags_;
    MftEntryInfo mft_record_;
    std::vector<other::IndexEntry> index_entries_;
};

} // namespace attributes
} // namespace mft
