#include "mft/attributes/object_id.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace attributes {

ObjectId::ObjectId(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    using common::ByteReader;
    
    object_id_guid_.fill(0);
    birth_volume_id_.fill(0);
    birth_object_id_.fill(0);
    domain_id_.fill(0);
    
    if (attribute_content_length_ >= 16) {
        ByteReader::block_copy(raw_bytes, content_offset_, 
                               object_id_guid_.data(), 0, 16);
    }
    
    if (attribute_content_length_ >= 32) {
        ByteReader::block_copy(raw_bytes, content_offset_ + 16,
                               birth_volume_id_.data(), 0, 16);
    }
    
    if (attribute_content_length_ >= 48) {
        ByteReader::block_copy(raw_bytes, content_offset_ + 32,
                               birth_object_id_.data(), 0, 16);
    }
    
    if (attribute_content_length_ >= 64) {
        ByteReader::block_copy(raw_bytes, content_offset_ + 48,
                               domain_id_.data(), 0, 16);
    }
}

std::string ObjectId::guid_to_string(const Guid& guid) {
    std::ostringstream oss;
    
    // Standard GUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    // But GUID bytes are in mixed-endian format: first 3 groups are little-endian
    
    // First 4 bytes (little-endian)
    oss << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(guid[3])
        << std::setw(2) << static_cast<int>(guid[2])
        << std::setw(2) << static_cast<int>(guid[1])
        << std::setw(2) << static_cast<int>(guid[0]) << "-";
    
    // Next 2 bytes (little-endian)
    oss << std::setw(2) << static_cast<int>(guid[5])
        << std::setw(2) << static_cast<int>(guid[4]) << "-";
    
    // Next 2 bytes (little-endian)
    oss << std::setw(2) << static_cast<int>(guid[7])
        << std::setw(2) << static_cast<int>(guid[6]) << "-";
    
    // Next 2 bytes (big-endian)
    oss << std::setw(2) << static_cast<int>(guid[8])
        << std::setw(2) << static_cast<int>(guid[9]) << "-";
    
    // Last 6 bytes (big-endian)
    for (int i = 10; i < 16; ++i) {
        oss << std::setw(2) << static_cast<int>(guid[i]);
    }
    
    return oss.str();
}

std::string ObjectId::object_id_string() const {
    return guid_to_string(object_id_guid_);
}

std::string ObjectId::birth_volume_id_string() const {
    return guid_to_string(birth_volume_id_);
}

std::string ObjectId::birth_object_id_string() const {
    return guid_to_string(birth_object_id_);
}

std::string ObjectId::domain_id_string() const {
    return guid_to_string(domain_id_);
}

std::string ObjectId::to_string() const {
    std::ostringstream oss;
    oss << "**** OBJECT ID ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    oss << "Object Id: " << object_id_string() << "\n";
    oss << "Birth Volume Id: " << birth_volume_id_string() << "\n";
    oss << "Birth Object Id: " << birth_object_id_string() << "\n";
    oss << "Domain Id: " << domain_id_string();
    return oss.str();
}

} // namespace attributes
} // namespace mft
