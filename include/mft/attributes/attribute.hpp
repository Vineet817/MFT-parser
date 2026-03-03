#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace mft {
namespace attributes {

/**
 * @brief NTFS Attribute types
 */
enum class AttributeType : int32_t {
    EndOfAttributes = -0x1,
    Unused = 0x0,
    StandardInformation = 0x10,
    AttributeList = 0x20,
    FileName = 0x30,
    VolumeVersionObjectId = 0x40,
    SecurityDescriptor = 0x50,
    VolumeName = 0x60,
    VolumeInformation = 0x70,
    Data = 0x80,
    IndexRoot = 0x90,
    IndexAllocation = 0xA0,
    Bitmap = 0xB0,
    ReparsePoint = 0xC0,
    ExtendedAttributeInformation = 0xD0,
    ExtendedAttribute = 0xE0,
    PropertySet = 0xF0,
    LoggedUtilityStream = 0x100,
    UserDefinedAttribute = 0x1000
};

/**
 * @brief Attribute data flags
 */
enum class AttributeDataFlag : uint16_t {
    None = 0x0000,
    Compressed = 0x0001,
    Encrypted = 0x4000,
    Sparse = 0x8000
};

// Allow bitwise operations on AttributeDataFlag
inline AttributeDataFlag operator|(AttributeDataFlag a, AttributeDataFlag b) {
    return static_cast<AttributeDataFlag>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

inline AttributeDataFlag operator&(AttributeDataFlag a, AttributeDataFlag b) {
    return static_cast<AttributeDataFlag>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

inline bool has_flag(AttributeDataFlag flags, AttributeDataFlag flag) {
    return (static_cast<uint16_t>(flags) & static_cast<uint16_t>(flag)) != 0;
}

/**
 * @brief Get string representation of attribute type
 */
std::string attribute_type_to_string(AttributeType type);

/**
 * @brief Get string representation of attribute data flags
 */
std::string attribute_data_flag_to_string(AttributeDataFlag flags);

/**
 * @brief Base class for all MFT attributes
 */
class Attribute {
public:
    /**
     * @brief Construct attribute from raw bytes
     * @param raw_bytes Pointer to attribute data
     * @param length Length of attribute data
     */
    Attribute(const uint8_t* raw_bytes, size_t length);
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Attribute() = default;
    
    // Accessors
    AttributeType attribute_type() const { return attribute_type_; }
    int32_t attribute_size() const { return attribute_size_; }
    int32_t attribute_content_length() const { return attribute_content_length_; }
    int name_size() const { return name_size_; }
    int name_offset() const { return name_offset_; }
    AttributeDataFlag attribute_data_flag() const { return attribute_data_flag_; }
    const std::string& name() const { return name_; }
    int16_t attribute_number() const { return attribute_number_; }
    bool is_resident() const { return is_resident_; }
    int16_t content_offset() const { return content_offset_; }
    
    /**
     * @brief Convert to string representation
     */
    virtual std::string to_string() const;
    
protected:
    AttributeType attribute_type_;
    int32_t attribute_size_;
    int32_t attribute_content_length_;
    int name_size_;
    int name_offset_;
    AttributeDataFlag attribute_data_flag_;
    std::string name_;
    int16_t attribute_number_;
    bool is_resident_;
    int16_t content_offset_;
    
    // Raw bytes stored for derived classes
    std::vector<uint8_t> raw_bytes_;
};

} // namespace attributes
} // namespace mft
