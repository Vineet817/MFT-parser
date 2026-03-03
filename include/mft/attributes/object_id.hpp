#pragma once

#include "mft/attributes/attribute.hpp"
#include "mft/common/datetime_utils.hpp"
#include <string>
#include <array>

namespace mft {
namespace attributes {

/**
 * @brief ObjectId attribute ($OBJECT_ID)
 * 
 * Contains GUIDs used to track files across moves/renames.
 */
class ObjectId : public Attribute {
public:
    using Guid = std::array<uint8_t, 16>;
    
    ObjectId(const uint8_t* raw_bytes, size_t length);
    
    const Guid& object_id_guid() const { return object_id_guid_; }
    const Guid& birth_volume_id() const { return birth_volume_id_; }
    const Guid& birth_object_id() const { return birth_object_id_; }
    const Guid& domain_id() const { return domain_id_; }
    
    std::string object_id_string() const;
    std::string birth_volume_id_string() const;
    std::string birth_object_id_string() const;
    std::string domain_id_string() const;
    
    std::string to_string() const override;

private:
    Guid object_id_guid_;
    Guid birth_volume_id_;
    Guid birth_object_id_;
    Guid domain_id_;
    
    static std::string guid_to_string(const Guid& guid);
};

} // namespace attributes
} // namespace mft
