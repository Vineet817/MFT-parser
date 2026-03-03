#pragma once

#include "mft/attributes/attribute.hpp"
#include <vector>

namespace mft {
namespace attributes {

/**
 * @brief SecurityDescriptor attribute ($SECURITY_DESCRIPTOR)
 * 
 * Contains security descriptor information.
 */
class SecurityDescriptor : public Attribute {
public:
    SecurityDescriptor(const uint8_t* raw_bytes, size_t length);
    
    std::string to_string() const override;
};

} // namespace attributes
} // namespace mft
