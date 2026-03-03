#pragma once

#include "mft/attributes/attribute.hpp"
#include <string>

namespace mft {
namespace attributes {

/**
 * @brief VolumeInformation attribute ($VOLUME_INFORMATION)
 */
class VolumeInformation : public Attribute {
public:
    VolumeInformation(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

/**
 * @brief LoggedUtilityStream attribute ($LOGGED_UTILITY_STREAM)
 */
class LoggedUtilityStream : public Attribute {
public:
    LoggedUtilityStream(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

/**
 * @brief ExtendedAttribute attribute ($EA)
 */
class ExtendedAttribute : public Attribute {
public:
    ExtendedAttribute(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

/**
 * @brief ExtendedAttributeInformation attribute ($EA_INFORMATION)
 */
class ExtendedAttributeInformation : public Attribute {
public:
    ExtendedAttributeInformation(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

/**
 * @brief PropertySet attribute ($PROPERTY_SET)
 */
class PropertySet : public Attribute {
public:
    PropertySet(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

/**
 * @brief UserDefinedAttribute
 */
class UserDefinedAttribute : public Attribute {
public:
    UserDefinedAttribute(const uint8_t* raw_bytes, size_t length);
    std::string to_string() const override;
};

} // namespace attributes
} // namespace mft
