#include "mft/attributes/other_attributes.hpp"
#include <sstream>

namespace mft {
namespace attributes {

// VolumeInformation
VolumeInformation::VolumeInformation(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string VolumeInformation::to_string() const {
    std::ostringstream oss;
    oss << "**** VOLUME INFORMATION ****\n" << Attribute::to_string();
    return oss.str();
}

// LoggedUtilityStream
LoggedUtilityStream::LoggedUtilityStream(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string LoggedUtilityStream::to_string() const {
    std::ostringstream oss;
    oss << "**** LOGGED UTILITY STREAM ****\n" << Attribute::to_string();
    return oss.str();
}

// ExtendedAttribute
ExtendedAttribute::ExtendedAttribute(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string ExtendedAttribute::to_string() const {
    std::ostringstream oss;
    oss << "**** EXTENDED ATTRIBUTE ****\n" << Attribute::to_string();
    return oss.str();
}

// ExtendedAttributeInformation
ExtendedAttributeInformation::ExtendedAttributeInformation(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string ExtendedAttributeInformation::to_string() const {
    std::ostringstream oss;
    oss << "**** EXTENDED ATTRIBUTE INFORMATION ****\n" << Attribute::to_string();
    return oss.str();
}

// PropertySet
PropertySet::PropertySet(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string PropertySet::to_string() const {
    std::ostringstream oss;
    oss << "**** PROPERTY SET ****\n" << Attribute::to_string();
    return oss.str();
}

// UserDefinedAttribute
UserDefinedAttribute::UserDefinedAttribute(const uint8_t* raw_bytes, size_t length) : Attribute(raw_bytes, length) {}
std::string UserDefinedAttribute::to_string() const {
    std::ostringstream oss;
    oss << "**** USER DEFINED ATTRIBUTE ****\n" << Attribute::to_string();
    return oss.str();
}

} // namespace attributes
} // namespace mft
