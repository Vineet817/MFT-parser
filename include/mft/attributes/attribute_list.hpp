#pragma once

#include "mft/attributes/attribute.hpp"
#include <vector>
#include <string>

namespace mft {
namespace attributes {

/**
 * @brief Attribute Loop Entry
 * 
 * Represents an entry in the Attribute List.
 */
class AttributeListEntry {
public:
    AttributeListEntry(const uint8_t* raw_bytes, size_t length);
    
    AttributeType type() const { return type_; }
    int16_t record_length() const { return record_length_; }
    int name_length() const { return name_length_; }
    int name_offset() const { return name_offset_; }
    uint64_t lowest_vcn() const { return lowest_vcn_; }
    uint64_t record_reference() const { return record_reference_; }
    int16_t attribute_number() const { return attribute_number_; }
    const std::string& name() const { return name_; }
    
    std::string to_string() const;

private:
    AttributeType type_;
    int16_t record_length_;
    int name_length_;
    int name_offset_;
    uint64_t lowest_vcn_;
    uint64_t record_reference_;
    int16_t attribute_number_;
    std::string name_;
};

/**
 * @brief AttributeList attribute ($ATTRIBUTE_LIST)
 * 
 * Used when an MFT record cannot contain all its attributes. 
 * Lists locators for attributes in other MFT records.
 */
class AttributeList : public Attribute {
public:
    AttributeList(const uint8_t* raw_bytes, size_t length);
    
    const std::vector<AttributeListEntry>& entries() const { return entries_; }
    
    std::string to_string() const override;

private:
    std::vector<AttributeListEntry> entries_;
};

} // namespace attributes
} // namespace mft
