#include "mft/attributes/security_descriptor.hpp"
#include <sstream>

namespace mft {
namespace attributes {

SecurityDescriptor::SecurityDescriptor(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    // Security Descriptor parsing is complex (SID, ACL, etc).
    // For now, we wrap the raw bytes.
}

std::string SecurityDescriptor::to_string() const {
    std::ostringstream oss;
    oss << "**** SECURITY DESCRIPTOR ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    // Future work: Decode Windows Security Descriptor format
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
