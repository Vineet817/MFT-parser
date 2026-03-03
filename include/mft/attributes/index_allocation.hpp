#pragma once

#include "mft/attributes/attribute.hpp"

namespace mft {
namespace attributes {

/**
 * @brief IndexAllocation attribute ($INDEX_ALLOCATION)
 * 
 * Contains the non-resident records of an index (e.g. large directories).
 */
class IndexAllocation : public Attribute {
public:
    IndexAllocation(const uint8_t* raw_bytes, size_t length);
    
    std::string to_string() const override;
};

} // namespace attributes
} // namespace mft
