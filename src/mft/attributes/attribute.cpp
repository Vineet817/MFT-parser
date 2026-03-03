#include "mft/attributes/attribute.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace attributes {

std::string attribute_type_to_string(AttributeType type) {
    switch (type) {
        case AttributeType::EndOfAttributes: return "EndOfAttributes";
        case AttributeType::Unused: return "Unused";
        case AttributeType::StandardInformation: return "StandardInformation";
        case AttributeType::AttributeList: return "AttributeList";
        case AttributeType::FileName: return "FileName";
        case AttributeType::VolumeVersionObjectId: return "VolumeVersionObjectId";
        case AttributeType::SecurityDescriptor: return "SecurityDescriptor";
        case AttributeType::VolumeName: return "VolumeName";
        case AttributeType::VolumeInformation: return "VolumeInformation";
        case AttributeType::Data: return "Data";
        case AttributeType::IndexRoot: return "IndexRoot";
        case AttributeType::IndexAllocation: return "IndexAllocation";
        case AttributeType::Bitmap: return "Bitmap";
        case AttributeType::ReparsePoint: return "ReparsePoint";
        case AttributeType::ExtendedAttributeInformation: return "ExtendedAttributeInformation";
        case AttributeType::ExtendedAttribute: return "ExtendedAttribute";
        case AttributeType::PropertySet: return "PropertySet";
        case AttributeType::LoggedUtilityStream: return "LoggedUtilityStream";
        case AttributeType::UserDefinedAttribute: return "UserDefinedAttribute";
        default: return "Unknown";
    }
}

std::string attribute_data_flag_to_string(AttributeDataFlag flags) {
    if (flags == AttributeDataFlag::None) {
        return "";
    }
    
    std::string result;
    if (has_flag(flags, AttributeDataFlag::Compressed)) {
        result += "Compressed";
    }
    if (has_flag(flags, AttributeDataFlag::Encrypted)) {
        if (!result.empty()) result += "|";
        result += "Encrypted";
    }
    if (has_flag(flags, AttributeDataFlag::Sparse)) {
        if (!result.empty()) result += "|";
        result += "Sparse";
    }
    return result;
}

Attribute::Attribute(const uint8_t* raw_bytes, size_t length) {
    using common::ByteReader;
    
    // Store raw bytes for derived classes
    raw_bytes_.assign(raw_bytes, raw_bytes + length);
    
    attribute_number_ = ByteReader::read_int16(raw_bytes, 0x0E);
    attribute_type_ = static_cast<AttributeType>(ByteReader::read_int32(raw_bytes, 0));
    attribute_size_ = ByteReader::read_int32(raw_bytes, 4);
    
    is_resident_ = raw_bytes[0x08] == 0;
    
    name_size_ = raw_bytes[0x09];
    name_offset_ = ByteReader::read_int16(raw_bytes, 0x0A);
    
    attribute_data_flag_ = static_cast<AttributeDataFlag>(ByteReader::read_int16(raw_bytes, 0x0C));
    
    attribute_content_length_ = ByteReader::read_int32(raw_bytes, 0x10);
    content_offset_ = ByteReader::read_int16(raw_bytes, 0x14);
    
    name_.clear();
    if (name_size_ > 0 && static_cast<size_t>(name_offset_ + name_size_ * 2) <= length) {
        name_ = ByteReader::read_unicode_string(raw_bytes, name_offset_, name_size_ * 2);
    }
}

std::string Attribute::to_string() const {
    std::ostringstream oss;
    
    oss << "Type: " << attribute_type_to_string(attribute_type_)
        << ", Attribute #: 0x" << std::uppercase << std::hex << attribute_number_;
    
    std::string flags_str = attribute_data_flag_to_string(attribute_data_flag_);
    if (!flags_str.empty()) {
        oss << ", Attribute flags: " << flags_str;
    }
    
    oss << ", Size: 0x" << attribute_size_
        << ", Content size: 0x" << attribute_content_length_
        << ", Name size: 0x" << name_size_;
    
    if (!name_.empty()) {
        oss << ", Name: " << name_;
    }
    
    oss << ", Content offset: 0x" << content_offset_
        << ", Resident: " << (is_resident_ ? "true" : "false");
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
