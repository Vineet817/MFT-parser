#include "mft/attributes/file_info.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace attributes {

std::string name_type_to_string(NameTypes type) {
    switch (type) {
        case NameTypes::Posix: return "Posix";
        case NameTypes::Windows: return "Windows";
        case NameTypes::Dos: return "Dos";
        case NameTypes::DosWindows: return "DosWindows";
        default: return "Unknown";
    }
}

FileInfo::FileInfo(const uint8_t* raw_bytes, size_t length)
    : physical_size_(0),
      logical_size_(0),
      flags_(StandardInfoFlag::None),
      reparse_value_(0),
      name_length_(0),
      name_type_(NameTypes::Posix)
{
    using common::ByteReader;
    using common::NullableDateTime;
    using common::Logger;
    
    // Parent MFT record (8 bytes at offset 0)
    parent_mft_record_ = MftEntryInfo(raw_bytes);
    
    // Timestamps
    int64_t created_raw = ByteReader::read_int64(raw_bytes, 0x08);
    if (created_raw > 0) {
        try {
            created_on_ = NullableDateTime::from_filetime(created_raw);
        } catch (...) {
            Logger::warning("Invalid CreatedOn timestamp");
        }
    }
    
    int64_t content_mod_raw = ByteReader::read_int64(raw_bytes, 0x10);
    if (content_mod_raw > 0) {
        try {
            content_modified_on_ = NullableDateTime::from_filetime(content_mod_raw);
        } catch (...) {
            Logger::warning("Invalid ContentModifiedOn timestamp");
        }
    }
    
    int64_t record_mod_raw = ByteReader::read_int64(raw_bytes, 0x18);
    if (record_mod_raw > 0) {
        try {
            record_modified_on_ = NullableDateTime::from_filetime(record_mod_raw);
        } catch (...) {
            Logger::warning("Invalid RecordModifiedOn timestamp");
        }
    }
    
    int64_t last_access_raw = ByteReader::read_int64(raw_bytes, 0x20);
    if (last_access_raw > 0) {
        try {
            last_accessed_on_ = NullableDateTime::from_filetime(last_access_raw);
        } catch (...) {
            Logger::warning("Invalid LastAccessedOn timestamp");
        }
    }
    
    physical_size_ = ByteReader::read_uint64(raw_bytes, 0x28);
    logical_size_ = ByteReader::read_uint64(raw_bytes, 0x30);
    
    flags_ = static_cast<StandardInfoFlag>(ByteReader::read_int32(raw_bytes, 0x38));
    reparse_value_ = ByteReader::read_int32(raw_bytes, 0x3C);
    
    name_length_ = raw_bytes[0x40];
    name_type_ = static_cast<NameTypes>(raw_bytes[0x41]);
    
    // File name is at offset 0x42, length is name_length_ * 2 (UTF-16)
    if (name_length_ > 0 && 0x42 + name_length_ * 2 <= length) {
        file_name_ = ByteReader::read_unicode_string(raw_bytes, 0x42, name_length_ * 2);
    }
}

std::string FileInfo::to_string() const {
    std::ostringstream oss;
    
    oss << "\nFile name: " << file_name_ 
        << " (Length: 0x" << std::hex << static_cast<int>(name_length_) << ")\n"
        << "Flags: " << standard_info_flag_to_string(flags_)
        << ", Name Type: " << name_type_to_string(name_type_)
        << ", Reparse Value: 0x" << reparse_value_
        << ", Physical Size: 0x" << physical_size_
        << ", Logical Size: 0x" << logical_size_ << "\n"
        << "Parent Mft Record: " << parent_mft_record_.to_string() << "\n\n"
        << std::dec
        << "Created On:\t\t" << created_on_.to_string() << "\n"
        << "Content Modified On:\t" << content_modified_on_.to_string() << "\n"
        << "Record Modified On:\t" << record_modified_on_.to_string() << "\n"
        << "Last Accessed On:\t" << last_accessed_on_.to_string();
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
