#include "mft/file_record.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include "mft/attributes/standard_info.hpp"
#include "mft/attributes/file_name.hpp"
#include "mft/attributes/data.hpp"
#include "mft/attributes/index_root.hpp"
#include "mft/attributes/reparse_point.hpp"
#include "mft/attributes/volume_name.hpp"
#include "mft/attributes/object_id.hpp"
#include "mft/attributes/attribute_list.hpp"
#include "mft/attributes/index_allocation.hpp"
#include "mft/attributes/bitmap.hpp"
#include "mft/attributes/security_descriptor.hpp"
#include "mft/attributes/other_attributes.hpp"
#include <sstream>
#include <iomanip>
#include <regex>

namespace mft {

std::string entry_flag_to_string(EntryFlag flags) {
    if (flags == EntryFlag::IsFree) {
        return "IsFree";
    }
    
    std::string result;
    auto append = [&result](const char* name) {
        if (!result.empty()) result += "|";
        result += name;
    };
    
    if (has_flag(flags, EntryFlag::InUse)) append("InUse");
    if (has_flag(flags, EntryFlag::IsDirectory)) append("IsDirectory");
    if (has_flag(flags, EntryFlag::IsMetaDataRecord)) append("IsMetaDataRecord");
    if (has_flag(flags, EntryFlag::IsIndexView)) append("IsIndexView");
    
    return result.empty() ? "IsFree" : result;
}

FileRecord::FileRecord(const uint8_t* raw_bytes, size_t length, int offset, bool recover_from_slack)
    : offset_(offset),
      entry_number_(0),
      sequence_number_(0),
      entry_flags_(EntryFlag::IsFree),
      log_sequence_number_(0),
      first_attribute_offset_(0),
      actual_record_size_(0),
      allocated_record_size_(0),
      first_available_attribute_id_(0),
      reference_count_(0),
      fixup_offset_(0),
      fixup_entry_count_(0),
      fixup_ok_(true),
      is_bad_(false),
      is_uninitialized_(false)
{
    using common::ByteReader;
    using common::Logger;
    
    int32_t sig = ByteReader::read_int32(raw_bytes, 0);
    
    switch (sig) {
        case FILE_SIG:
            break;
        case BAAD_SIG:
            Logger::debug_fmt("Bad signature at offset 0x{:X}", offset);
            is_bad_ = true;
            return;
        default:
            Logger::debug_fmt("Uninitialized entry (no signature) at offset 0x{:X}", offset);
            is_uninitialized_ = true;
            return;
    }
    
    Logger::debug_fmt("Processing FILE record at offset 0x{:X}", offset);
    
    fixup_offset_ = ByteReader::read_int16(raw_bytes, 0x04);
    fixup_entry_count_ = ByteReader::read_int16(raw_bytes, 0x06);
    
    // Build fixup data
    size_t fixup_total_length = static_cast<size_t>(fixup_entry_count_) * 2;
    std::vector<uint8_t> fixup_buffer(fixup_total_length);
    ByteReader::block_copy(raw_bytes, fixup_offset_, fixup_buffer.data(), 0, fixup_total_length);
    
    // Pull entry flags early for fixup verification
    entry_flags_ = static_cast<EntryFlag>(ByteReader::read_int16(raw_bytes, 0x16));
    
    fixup_data_ = FixupData(fixup_buffer.data(), fixup_total_length);
    
    // Make a mutable copy for fixup application
    std::vector<uint8_t> mutable_bytes(raw_bytes, raw_bytes + length);
    
    // Fixup verification and application
    int counter = 512;
    for (const auto& actual_bytes : fixup_data_.fixup_actual()) {
        int fix_offset = counter - 2;
        
        int16_t expected = ByteReader::read_int16(mutable_bytes.data(), fix_offset);
        if (expected != fixup_data_.fixup_expected() && entry_flags_ != EntryFlag::IsFree) {
            fixup_ok_ = false;
            Logger::warning_fmt(
                "Offset: 0x{:X} Entry/seq: 0x{:X}/0x{:X} Fixup values do not match at 0x{:X}. "
                "Expected: 0x{:X}, actual: 0x{:X}",
                offset_, entry_number_, sequence_number_, fix_offset, 
                fixup_data_.fixup_expected(), expected);
        }
        
        // Replace fixup expected with actual bytes
        ByteReader::block_copy(actual_bytes.data(), 0, mutable_bytes.data(), fix_offset, 2);
        counter += 512;
    }
    
    log_sequence_number_ = ByteReader::read_int64(mutable_bytes.data(), 0x08);
    sequence_number_ = ByteReader::read_uint16(mutable_bytes.data(), 0x10);
    reference_count_ = ByteReader::read_int16(mutable_bytes.data(), 0x12);
    first_attribute_offset_ = ByteReader::read_int16(mutable_bytes.data(), 0x14);
    actual_record_size_ = ByteReader::read_int32(mutable_bytes.data(), 0x18);
    allocated_record_size_ = ByteReader::read_int32(mutable_bytes.data(), 0x1C);
    
    uint8_t entry_bytes[8];
    ByteReader::block_copy(mutable_bytes.data(), 0x20, entry_bytes, 0, 8);
    mft_record_to_base_record_ = MftEntryInfo(entry_bytes);
    
    first_available_attribute_id_ = ByteReader::read_int16(mutable_bytes.data(), 0x28);
    entry_number_ = ByteReader::read_uint32(mutable_bytes.data(), 0x2C);
    
    Logger::debug_fmt("FILE record entry/seq #: 0x{:X}/{:X}", entry_number_, sequence_number_);
    
    // Parse attributes
    size_t index = static_cast<size_t>(first_attribute_offset_);
    
    while (index < static_cast<size_t>(actual_record_size_) && index < length) {
        auto attr_type = static_cast<attributes::AttributeType>(
            ByteReader::read_int32(mutable_bytes.data(), index));
        int32_t attr_size = ByteReader::read_int32(mutable_bytes.data(), index + 4);
        
        if (attr_size == 0 || attr_type == attributes::AttributeType::EndOfAttributes) {
            index += 8;
            break;
        }
        
        Logger::debug_fmt("Found Attribute Type {} at absolute offset: 0x{:X}",
            attributes::attribute_type_to_string(attr_type), index + offset);
        
        parse_attribute(mutable_bytes.data() + index, attr_size, attr_type, index + offset);
        
        index += static_cast<size_t>(attr_size);
    }
    
    Logger::verbose_fmt("Slack starts at 0x{:X} Absolute offset: 0x{:X}", index, index + offset);
    
    // Slack space recovery would go here if recover_from_slack is true
    (void)recover_from_slack;  // TODO: Implement slack recovery
}

void FileRecord::parse_attribute(const uint8_t* raw_bytes, size_t length,
                                 attributes::AttributeType type, int abs_offset)
{
    using namespace attributes;
    using common::Logger;
    
    std::unique_ptr<Attribute> attr;
    
    try {
        switch (type) {
            case AttributeType::StandardInformation:
                attr = std::make_unique<StandardInfo>(raw_bytes, length);
                break;
            case AttributeType::FileName:
                attr = std::make_unique<FileName>(raw_bytes, length);
                break;
            case AttributeType::Data:
                attr = std::make_unique<Data>(raw_bytes, length);
                break;
            case AttributeType::IndexRoot:
                attr = std::make_unique<IndexRoot>(raw_bytes, length);
                break;
            case AttributeType::ReparsePoint:
                attr = std::make_unique<ReparsePoint>(raw_bytes, length);
                break;
            case AttributeType::VolumeName:
                attr = std::make_unique<VolumeName>(raw_bytes, length);
                break;
            case AttributeType::VolumeVersionObjectId:
                attr = std::make_unique<ObjectId>(raw_bytes, length);
                break;
            case AttributeType::AttributeList:
                attr = std::make_unique<AttributeList>(raw_bytes, length);
                break;
            case AttributeType::IndexAllocation:
                attr = std::make_unique<IndexAllocation>(raw_bytes, length);
                break;
            case AttributeType::Bitmap:
                attr = std::make_unique<Bitmap>(raw_bytes, length);
                break;
            case AttributeType::SecurityDescriptor:
                attr = std::make_unique<SecurityDescriptor>(raw_bytes, length);
                break;
            case AttributeType::VolumeInformation:
                attr = std::make_unique<VolumeInformation>(raw_bytes, length);
                break;
            case AttributeType::LoggedUtilityStream:
                attr = std::make_unique<LoggedUtilityStream>(raw_bytes, length);
                break;
            case AttributeType::ExtendedAttribute:
                attr = std::make_unique<ExtendedAttribute>(raw_bytes, length);
                break;
            case AttributeType::ExtendedAttributeInformation:
                attr = std::make_unique<ExtendedAttributeInformation>(raw_bytes, length);
                break;
            case AttributeType::PropertySet:
                attr = std::make_unique<PropertySet>(raw_bytes, length);
                break;
            // Note: UserDefinedAttribute usually doesn't have a fixed type ID in standard enum
            default:
                // For unimplemented types, create base Attribute
                attr = std::make_unique<Attribute>(raw_bytes, length);
                break;
        }
        
        if (attr) {
            attributes_.push_back(std::move(attr));
        }
    } catch (const std::exception& e) {
        Logger::error_fmt("Error parsing attribute at offset 0x{:X}: {}", abs_offset, e.what());
    }
}

bool FileRecord::is_directory() const {
    return has_flag(entry_flags_, EntryFlag::IsDirectory);
}

bool FileRecord::is_deleted() const {
    return !has_flag(entry_flags_, EntryFlag::InUse);
}

std::string FileRecord::get_key(bool as_decimal) const {
    std::ostringstream oss;
    
    uint16_t seq = sequence_number_;
    if (is_deleted() && seq > 0) {
        seq -= 1;
    }
    
    if (as_decimal) {
        oss << entry_number_ << "-" << seq;
    } else {
        oss << std::uppercase << std::hex << std::setfill('0')
            << std::setw(8) << entry_number_ << "-"
            << std::setw(8) << seq;
    }
    
    return oss.str();
}

std::string FileRecord::to_string() const {
    std::ostringstream oss;
    
    oss << std::hex << std::uppercase
        << "Entry-seq #: 0x" << entry_number_ << "-0x" << sequence_number_
        << ", Offset: 0x" << offset_
        << ", Flags: " << entry_flag_to_string(entry_flags_)
        << ", Log Sequence #: 0x" << log_sequence_number_
        << ", Mft Record To Base Record: " << mft_record_to_base_record_.to_string()
        << "\nReference Count: 0x" << reference_count_
        << ", Fixup Data: " << fixup_data_.to_string()
        << " (Fixup OK: " << (fixup_ok_ ? "true" : "false") << ")\n\n";
    
    for (const auto& attr : attributes_) {
        oss << attr->to_string() << "\n";
    }
    
    return oss.str();
}

std::vector<std::unique_ptr<other::IndexEntryI30>> FileRecord::get_slack_file_entries(
    const uint8_t* slack_space, size_t length,
    int page_number, int start_offset, uint32_t entry_number)
{
    // TODO: Implement slack recovery
    (void)slack_space;
    (void)length;
    (void)page_number;
    (void)start_offset;
    (void)entry_number;
    
    return {};
}

} // namespace mft
