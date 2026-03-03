#include "mft/fixup_data.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {

FixupData::FixupData(const uint8_t* fixup_data_raw, size_t length) {
    using common::ByteReader;
    
    fixup_expected_ = ByteReader::read_int16(fixup_data_raw, 0);
    
    size_t index = 2;
    while (index < length) {
        std::vector<uint8_t> bytes(2);
        bytes[0] = fixup_data_raw[index];
        bytes[1] = fixup_data_raw[index + 1];
        fixup_actual_.push_back(std::move(bytes));
        index += 2;
    }
}

std::string FixupData::to_string() const {
    std::ostringstream oss;
    
    // Format expected value
    oss << "Expected: ";
    oss << std::uppercase << std::hex << std::setfill('0');
    
    uint8_t expected_bytes[2];
    expected_bytes[0] = static_cast<uint8_t>(fixup_expected_ & 0xFF);
    expected_bytes[1] = static_cast<uint8_t>((fixup_expected_ >> 8) & 0xFF);
    oss << std::setw(2) << static_cast<int>(expected_bytes[0]) << "-"
        << std::setw(2) << static_cast<int>(expected_bytes[1]);
    
    oss << " Fixup Actual: ";
    
    for (size_t i = 0; i < fixup_actual_.size(); ++i) {
        if (i > 0) {
            oss << "|";
        }
        oss << std::setw(2) << static_cast<int>(fixup_actual_[i][0]) << "-"
            << std::setw(2) << static_cast<int>(fixup_actual_[i][1]);
    }
    
    return oss.str();
}

} // namespace mft
