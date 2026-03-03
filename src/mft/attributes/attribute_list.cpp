#include "mft/attributes/attribute_list.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace attributes {

AttributeListEntry::AttributeListEntry(const uint8_t* raw_bytes, size_t length) {
    using common::ByteReader;
    
    type_ = static_cast<AttributeType>(ByteReader::read_int32(raw_bytes, 0));
    record_length_ = ByteReader::read_int16(raw_bytes, 4);
    name_length_ = raw_bytes[6];
    name_offset_ = raw_bytes[7];
    lowest_vcn_ = ByteReader::read_uint64(raw_bytes, 8);
    record_reference_ = ByteReader::read_uint64(raw_bytes, 16);
    attribute_number_ = ByteReader::read_int16(raw_bytes, 24);
    
    if (name_length_ > 0 && name_offset_ + name_length_ * 2 <= length) {
        name_ = ByteReader::read_unicode_string(raw_bytes, name_offset_, name_length_ * 2);
    }
}

std::string AttributeListEntry::to_string() const {
    std::ostringstream oss;
    oss << "Type: " << attribute_type_to_string(type_)
        << ", Reference: 0x" << std::hex << record_reference_
        << ", VCN: 0x" << lowest_vcn_
        << ", Attribute #: 0x" << attribute_number_;
    
    if (!name_.empty()) {
        oss << ", Name: " << name_;
    }
    
    return oss.str();
}

AttributeList::AttributeList(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    using common::ByteReader;
    
    // Attribute List can be resident or non-resident
    if (is_resident_) {
        size_t offset = content_offset_;
        size_t end = offset + attribute_content_length_;
        
        while (offset < end && offset < length) {
            AttributeType type = static_cast<AttributeType>(ByteReader::read_int32(raw_bytes, offset));
            int16_t rec_len = ByteReader::read_int16(raw_bytes, offset + 4);
            
            if (rec_len == 0) break;
            
            if (offset + rec_len > length) break;
            
            entries_.emplace_back(raw_bytes + offset, rec_len);
            
            offset += rec_len;
        }
    } else {
         // Non-resident attribute list needs to be read from disk clusters
         // The Attribute class parses the non-resident header, but we don't have access to the raw data here
         // This is typically handled by the MftFile class resolving the data runs
         // For now, we just acknowledge it exists.
    }
}

std::string AttributeList::to_string() const {
    std::ostringstream oss;
    oss << "**** ATTRIBUTE LIST ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    for (const auto& entry : entries_) {
        oss << entry.to_string() << "\n";
    }
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
