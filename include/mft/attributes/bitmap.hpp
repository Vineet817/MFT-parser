#pragma once

#include "mft/attributes/attribute.hpp"
#include <vector>

namespace mft {
namespace attributes {

/**
 * @brief Bitmap attribute ($BITMAP)
 * 
 * Bitfield representing allocation status of index entries or MFT records.
 */
class Bitmap : public Attribute {
public:
    Bitmap(const uint8_t* raw_bytes, size_t length);
    
    bool bit_set(int bit_index) const;
    
    std::string to_string() const override;

private:
    std::vector<uint8_t> bitmap_data_;
};

} // namespace attributes
} // namespace mft
