#include "mft/attributes/index_root.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace other {

IndexEntry::IndexEntry(const uint8_t* raw_bytes, size_t length)
    : physical_size_(0),
      logical_size_(0),
      flags_(attributes::StandardInfoFlag::None),
      reparse_value_(0),
      name_length_(0),
      name_type_(attributes::NameTypes::Posix)
{
    using common::ByteReader;
    using common::NullableDateTime;
    using common::Logger;
    
    size_t index = 0;
    index += 2;  // size
    
    int16_t index_key_data_size = ByteReader::read_int16(raw_bytes, index);
    index += 2;
    
    auto index_flags = static_cast<attributes::IndexFlag>(ByteReader::read_int32(raw_bytes, index));
    index += 4;
    
    if (attributes::has_flag(index_flags, attributes::IndexFlag::IsLast)) {
        return;
    }
    
    if (index_key_data_size == 0x10 || index_key_data_size <= 0x40) {
        return;
    }
    
    if (index_key_data_size > 0 && index + 8 <= length) {
        parent_mft_record_ = MftEntryInfo(raw_bytes + index);
        index += 8;
        
        if (index + 8 <= length) {
            int64_t created_raw = ByteReader::read_int64(raw_bytes, index);
            if (created_raw > 0) {
                try {
                    created_on_ = NullableDateTime::from_filetime(created_raw);
                } catch (...) {
                    Logger::warning("Invalid CreatedOn timestamp in IndexEntry");
                }
            }
        }
        index += 8;
        
        if (index + 8 <= length) {
            int64_t content_mod_raw = ByteReader::read_int64(raw_bytes, index);
            if (content_mod_raw > 0) {
                try {
                    content_modified_on_ = NullableDateTime::from_filetime(content_mod_raw);
                } catch (...) {}
            }
        }
        index += 8;
        
        if (index + 8 <= length) {
            int64_t record_mod_raw = ByteReader::read_int64(raw_bytes, index);
            if (record_mod_raw > 0) {
                try {
                    record_modified_on_ = NullableDateTime::from_filetime(record_mod_raw);
                } catch (...) {}
            }
        }
        index += 8;
        
        if (index + 8 <= length) {
            int64_t last_access_raw = ByteReader::read_int64(raw_bytes, index);
            if (last_access_raw > 0) {
                try {
                    last_accessed_on_ = NullableDateTime::from_filetime(last_access_raw);
                } catch (...) {}
            }
        }
        index += 8;
        
        if (index + 16 <= length) {
            physical_size_ = ByteReader::read_uint64(raw_bytes, index);
            index += 8;
            logical_size_ = ByteReader::read_uint64(raw_bytes, index);
            index += 8;
        }
        
        if (index + 8 <= length) {
            flags_ = static_cast<attributes::StandardInfoFlag>(ByteReader::read_int32(raw_bytes, index));
            index += 4;
            reparse_value_ = ByteReader::read_int32(raw_bytes, index);
            index += 4;
        }
        
        if (index + 2 <= length) {
            name_length_ = raw_bytes[index];
            index += 1;
            name_type_ = static_cast<attributes::NameTypes>(raw_bytes[index]);
            index += 1;
            
            if (name_length_ > 0 && index + name_length_ * 2 <= length) {
                file_name_ = ByteReader::read_unicode_string(raw_bytes, index, name_length_ * 2);
            }
        }
    }
}

std::string IndexEntry::to_string() const {
    std::ostringstream oss;
    oss << "\nFile name: " << file_name_ 
        << " (Len:0x" << std::hex << static_cast<int>(name_length_) << ")"
        << " Flags: " << attributes::standard_info_flag_to_string(flags_)
        << ", Name Type: " << attributes::name_type_to_string(name_type_)
        << " Reparse Value: 0x" << reparse_value_
        << " Physical Size: 0x" << physical_size_
        << ", Logical Size: 0x" << logical_size_
        << "\nParent Mft Record: " << parent_mft_record_.to_string()
        << std::dec
        << "\nCreated On:\t\t" << created_on_.to_string()
        << "\nContent Modified On:\t" << content_modified_on_.to_string()
        << "\nRecord Modified On:\t" << record_modified_on_.to_string()
        << "\nLast Accessed On:\t" << last_accessed_on_.to_string();
    return oss.str();
}

} // namespace other

namespace attributes {

IndexRoot::IndexRoot(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length),
      indexed_attribute_type_(AttributeType::Unused),
      collation_type_(CollationType::Binary),
      entry_size_(0),
      number_cluster_blocks_(0),
      offset_to_first_index_entry_(0),
      total_size_of_index_entries_(0),
      allocated_size_of_entries_(0),
      flags_(IndexFlag::None)
{
    using common::ByteReader;
    
    size_t index = static_cast<size_t>(content_offset_);
    
    indexed_attribute_type_ = static_cast<AttributeType>(ByteReader::read_int32(raw_bytes, index));
    index += 4;
    
    collation_type_ = static_cast<CollationType>(ByteReader::read_int32(raw_bytes, index));
    index += 4;
    
    entry_size_ = ByteReader::read_int32(raw_bytes, index);
    index += 4;
    
    number_cluster_blocks_ = ByteReader::read_int32(raw_bytes, index);
    index += 4;
    
    offset_to_first_index_entry_ = ByteReader::read_int32(raw_bytes, index);
    index += 4;
    
    total_size_of_index_entries_ = ByteReader::read_int32(raw_bytes, index);
    index += 4;
    
    allocated_size_of_entries_ = ByteReader::read_int32(raw_bytes, index);
    index += 4;
    
    flags_ = static_cast<IndexFlag>(raw_bytes[index]);
    index += 1;
    
    index += 3;  // padding
    
    // MFT record info
    if (index + 8 <= length) {
        mft_record_ = MftEntryInfo(raw_bytes + index);
        index += 8;
    }
    
    // Parse index entries
    while (index < length) {
        int16_t index_val_size = ByteReader::read_int16(raw_bytes, index);
        
        if (index_val_size == 0x10) {
            break;
        }
        
        if (index_val_size > static_cast<int16_t>(length - index)) {
            index_val_size = static_cast<int16_t>(length - index);
        }
        
        if (index_val_size > 0) {
            index_entries_.emplace_back(raw_bytes + index, static_cast<size_t>(index_val_size));
        }
        
        index += static_cast<size_t>(index_val_size);
    }
}

std::string IndexRoot::to_string() const {
    std::ostringstream oss;
    
    oss << "**** INDEX ROOT ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    oss << std::hex << std::uppercase
        << "Indexed Attribute Type: " << attribute_type_to_string(indexed_attribute_type_)
        << " Entry Size: 0x" << entry_size_
        << " Number Cluster Blocks: 0x" << number_cluster_blocks_
        << " Collation Type: " << static_cast<int>(collation_type_)
        << " Index entries count: 0x" << index_entries_.size()
        << " Mft Record: " << mft_record_.to_string() << "\n\n"
        << std::dec;
    
    oss << "FileInfo Records Entries\n";
    for (const auto& ie : index_entries_) {
        oss << ie.to_string() << "\n";
    }
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
