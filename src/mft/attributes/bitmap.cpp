#include "mft/attributes/bitmap.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>

namespace mft {
namespace attributes {

Bitmap::Bitmap(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    if (is_resident_) {
        bitmap_data_.assign(raw_bytes + content_offset_, 
                           raw_bytes + content_offset_ + attribute_content_length_);
    }
    // Non-resident bitmap needs to be read from disk
}

bool Bitmap::bit_set(int bit_index) const {
    if (bit_index < 0) return false;
    
    size_t byte_idx = bit_index / 8;
    int bit_offset = bit_index % 8;
    
    if (byte_idx < bitmap_data_.size()) {
        return (bitmap_data_[byte_idx] & (1 << bit_offset)) != 0;
    }
    return false;
}

std::string Bitmap::to_string() const {
    std::ostringstream oss;
    oss << "**** BITMAP ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    if (is_resident_) {
        oss << "Bitmap size: " << bitmap_data_.size() << " bytes\n";
    }
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
