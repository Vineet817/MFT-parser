#include "mft/other/index_entry_i30.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace other {

IndexEntryI30::IndexEntryI30(const uint8_t* raw_bytes, size_t length,
                             int64_t absolute_offset, int page_number, bool from_slack)
    : flag_(OEntryFlag::HasSubNodes),
      page_number_(page_number),
      from_slack_(from_slack),
      absolute_offset_(absolute_offset)
{
    using common::ByteReader;
    
    size_t skip_offset = 0;
    
    if (!from_slack) {
        if (length < 8) return;
        
        mft_reference_self_ = MftEntryInfo(raw_bytes);
        
        if (mft_reference_self_.mft_entry_number() == 0) {
            return;
        }
        
        int16_t index_entry_size = ByteReader::read_int16(raw_bytes, 8);
        (void)index_entry_size;  // Used for validation
        int16_t index_data_size = ByteReader::read_int16(raw_bytes, 10);
        (void)index_data_size;
        
        flag_ = static_cast<OEntryFlag>(ByteReader::read_int32(raw_bytes, 12));
        skip_offset = 8 + 2 + 2 + 4;  // 16 bytes
    }
    
    if (skip_offset < length) {
        file_info_ = std::make_unique<attributes::FileInfo>(
            raw_bytes + skip_offset, length - skip_offset);
    }
}

std::string IndexEntryI30::to_string() const {
    std::ostringstream oss;
    
    oss << "Absolute offset: 0x" << std::hex << absolute_offset_
        << " FromSlack: " << (from_slack_ ? "true" : "false")
        << " Self MFT: " << mft_reference_self_.to_string();
    
    if (file_info_) {
        oss << " FileInfo: " << file_info_->to_string();
    }
    
    return oss.str();
}

} // namespace other
} // namespace mft
