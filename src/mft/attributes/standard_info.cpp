#include "mft/attributes/standard_info.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace attributes {

std::string standard_info_flag_to_string(StandardInfoFlag flags) {
    if (flags == StandardInfoFlag::None) {
        return "None";
    }
    
    std::string result;
    auto append = [&result](const char* name) {
        if (!result.empty()) result += "|";
        result += name;
    };
    
    if (has_flag(flags, StandardInfoFlag::ReadOnly)) append("ReadOnly");
    if (has_flag(flags, StandardInfoFlag::Hidden)) append("Hidden");
    if (has_flag(flags, StandardInfoFlag::System)) append("System");
    if (has_flag(flags, StandardInfoFlag::VolumeLabel)) append("VolumeLabel");
    if (has_flag(flags, StandardInfoFlag::Directory)) append("Directory");
    if (has_flag(flags, StandardInfoFlag::Archive)) append("Archive");
    if (has_flag(flags, StandardInfoFlag::Device)) append("Device");
    if (has_flag(flags, StandardInfoFlag::Normal)) append("Normal");
    if (has_flag(flags, StandardInfoFlag::Temporary)) append("Temporary");
    if (has_flag(flags, StandardInfoFlag::SparseFile)) append("SparseFile");
    if (has_flag(flags, StandardInfoFlag::ReparsePoint)) append("ReparsePoint");
    if (has_flag(flags, StandardInfoFlag::Compressed)) append("Compressed");
    if (has_flag(flags, StandardInfoFlag::Offline)) append("Offline");
    if (has_flag(flags, StandardInfoFlag::NotContentIndexed)) append("NotContentIndexed");
    if (has_flag(flags, StandardInfoFlag::Encrypted)) append("Encrypted");
    if (has_flag(flags, StandardInfoFlag::IntegrityStream)) append("IntegrityStream");
    if (has_flag(flags, StandardInfoFlag::Virtual)) append("Virtual");
    if (has_flag(flags, StandardInfoFlag::NoScrubData)) append("NoScrubData");
    if (has_flag(flags, StandardInfoFlag::RecallOnOpen)) append("RecallOnOpen");
    if (has_flag(flags, StandardInfoFlag::RecallOnDataAccess)) append("RecallOnDataAccess");
    if (has_flag(flags, StandardInfoFlag::Pinned)) append("Pinned");
    if (has_flag(flags, StandardInfoFlag::UnPinned)) append("UnPinned");
    if (has_flag(flags, StandardInfoFlag::IsDirectory)) append("IsDirectory");
    if (has_flag(flags, StandardInfoFlag::IsIndexView)) append("IsIndexView");
    
    return result.empty() ? "None" : result;
}

StandardInfo::StandardInfo(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length),
      flags_(StandardInfoFlag::None),
      flags2_(StandardInfoFlag2::None),
      max_version_(0),
      class_id_(0),
      owner_id_(0),
      security_id_(0),
      quota_charged_(0),
      update_sequence_number_(0)
{
    using common::ByteReader;
    using common::NullableDateTime;
    using common::Logger;
    
    // Timestamps start at offset 0x18 (after attribute header)
    int64_t created_raw = ByteReader::read_int64(raw_bytes, 0x18);
    if (created_raw > 0) {
        try {
            created_on_ = NullableDateTime::from_filetime(created_raw);
        } catch (...) {
            Logger::warning("Invalid CreatedOn timestamp");
        }
    }
    
    int64_t content_mod_raw = ByteReader::read_int64(raw_bytes, 0x20);
    if (content_mod_raw > 0) {
        try {
            content_modified_on_ = NullableDateTime::from_filetime(content_mod_raw);
        } catch (...) {
            Logger::warning("Invalid ContentModifiedOn timestamp");
        }
    }
    
    int64_t record_mod_raw = ByteReader::read_int64(raw_bytes, 0x28);
    if (record_mod_raw > 0) {
        try {
            record_modified_on_ = NullableDateTime::from_filetime(record_mod_raw);
        } catch (...) {
            Logger::warning("Invalid RecordModifiedOn timestamp");
        }
    }
    
    int64_t last_access_raw = ByteReader::read_int64(raw_bytes, 0x30);
    if (last_access_raw > 0) {
        try {
            last_accessed_on_ = NullableDateTime::from_filetime(last_access_raw);
        } catch (...) {
            Logger::warning("Invalid LastAccessedOn timestamp");
        }
    }
    
    flags_ = static_cast<StandardInfoFlag>(ByteReader::read_int32(raw_bytes, 0x38));
    
    max_version_ = ByteReader::read_int32(raw_bytes, 0x3C);
    flags2_ = static_cast<StandardInfoFlag2>(ByteReader::read_int32(raw_bytes, 0x40));
    class_id_ = ByteReader::read_int32(raw_bytes, 0x44);
    
    // Extended attributes (may not be present in older NTFS)
    if (length > 0x48) {
        owner_id_ = ByteReader::read_int32(raw_bytes, 0x48);
        security_id_ = ByteReader::read_int32(raw_bytes, 0x4C);
        quota_charged_ = ByteReader::read_int32(raw_bytes, 0x50);
        update_sequence_number_ = ByteReader::read_int64(raw_bytes, 0x58);
    }
}

std::string StandardInfo::to_string() const {
    std::ostringstream oss;
    
    oss << "**** STANDARD INFO ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    oss << "Flags: " << standard_info_flag_to_string(flags_)
        << ", Max Version: 0x" << std::hex << max_version_
        << ", Flags 2: " << (flags2_ == StandardInfoFlag2::IsCaseSensitive ? "IsCaseSensitive" : "None")
        << ", Class Id: 0x" << class_id_ << "\n"
        << "Owner Id: 0x" << owner_id_
        << ", Security Id: 0x" << security_id_
        << ", Quota Charged: 0x" << quota_charged_ << "\n"
        << "Update Sequence #: 0x" << update_sequence_number_ << "\n\n";
    
    oss << std::dec;
    oss << "Created On:\t\t" << created_on_.to_string() << "\n";
    oss << "Content Modified On:\t" << content_modified_on_.to_string() << "\n";
    oss << "Record Modified On:\t" << record_modified_on_.to_string() << "\n";
    oss << "Last Accessed On:\t" << last_accessed_on_.to_string();
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
