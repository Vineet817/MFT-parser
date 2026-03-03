#include "mft/attributes/volume_name.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>

namespace mft {
namespace attributes {

VolumeName::VolumeName(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    using common::ByteReader;
    
    if (attribute_content_length_ > 0) {
        volume_name_ = ByteReader::read_unicode_string(
            raw_bytes, content_offset_, attribute_content_length_);
    }
}

std::string VolumeName::to_string() const {
    std::ostringstream oss;
    oss << "**** VOLUME NAME ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    oss << "Volume Name: " << volume_name_;
    return oss.str();
}

} // namespace attributes
} // namespace mft
