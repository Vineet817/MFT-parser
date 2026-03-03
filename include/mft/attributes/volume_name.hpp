#pragma once

#include "mft/attributes/attribute.hpp"
#include <string>

namespace mft {
namespace attributes {

/**
 * @brief VolumeName attribute ($VOLUME_NAME)
 * 
 * Contains the name of the volume.
 */
class VolumeName : public Attribute {
public:
    VolumeName(const uint8_t* raw_bytes, size_t length);
    
    const std::string& volume_name() const { return volume_name_; }
    
    std::string to_string() const override;

private:
    std::string volume_name_;
};

} // namespace attributes
} // namespace mft
