#include "mft/mft_entry_info.hpp"
#include "mft/common/byte_reader.hpp"

namespace mft {

MftEntryInfo::MftEntryInfo(const uint8_t* raw_bytes) {
    using common::ByteReader;
    
    // Entry number is in bytes 0-5 (48 bits, but we use 32 + 16 calculation)
    // Sequence number is in bytes 6-7
    
    uint16_t sequence_number = ByteReader::read_uint16(raw_bytes, 6);
    
    uint32_t entry_index1 = ByteReader::read_uint32(raw_bytes, 0);
    uint16_t entry_index2 = ByteReader::read_uint16(raw_bytes, 4);
    
    uint32_t entry_index;
    if (entry_index2 == 0) {
        entry_index = entry_index1;
    } else {
        // Combine the high 16 bits
        uint32_t high_part = static_cast<uint32_t>(entry_index2) * 16777216; // 2^24
        entry_index = entry_index1 + high_part;
    }
    
    mft_entry_number_ = entry_index;
    mft_sequence_number_ = sequence_number;
}

std::string MftEntryInfo::get_key(bool as_decimal) const {
    std::ostringstream oss;
    
    if (as_decimal) {
        oss << mft_entry_number_ << "-" << mft_sequence_number_;
    } else {
        oss << std::uppercase << std::hex << std::setfill('0')
            << std::setw(8) << mft_entry_number_ << "-"
            << std::setw(8) << mft_sequence_number_;
    }
    
    return oss.str();
}

std::string MftEntryInfo::to_string() const {
    std::ostringstream oss;
    oss << "Entry/seq: 0x" << std::uppercase << std::hex 
        << mft_entry_number_ << "-0x" << mft_sequence_number_;
    return oss.str();
}

} // namespace mft
