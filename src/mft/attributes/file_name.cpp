#include "mft/attributes/file_name.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>

namespace mft {
namespace attributes {

FileName::FileName(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length),
      file_info_(raw_bytes + content_offset_, length - content_offset_)
{
}

std::string FileName::to_string() const {
    std::ostringstream oss;
    
    oss << "**** FILE NAME ****\n";
    oss << this->Attribute::to_string() << "\n";
    oss << file_info_.to_string();
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
