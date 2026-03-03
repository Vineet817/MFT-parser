#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace mft {

/**
 * @brief Fixup data for sector validation
 * 
 * NTFS uses fixup values at the end of each 512-byte sector to detect
 * incomplete writes. This class parses and applies the fixup.
 */
class FixupData {
public:
    /**
     * @brief Construct from raw fixup data
     * @param fixup_data_raw Raw fixup bytes (expected value + replacement pairs)
     * @param length Length of the fixup data
     */
    FixupData(const uint8_t* fixup_data_raw, size_t length);
    
    /**
     * @brief Default constructor
     */
    FixupData() : fixup_expected_(0) {}
    
    /**
     * @brief Get the expected fixup value at end of each sector
     */
    int16_t fixup_expected() const { return fixup_expected_; }
    
    /**
     * @brief Get the actual replacement bytes for each sector
     * @return Vector of 2-byte arrays to overlay
     */
    const std::vector<std::vector<uint8_t>>& fixup_actual() const { 
        return fixup_actual_; 
    }
    
    /**
     * @brief Convert to string representation
     */
    std::string to_string() const;

private:
    int16_t fixup_expected_;
    std::vector<std::vector<uint8_t>> fixup_actual_;
};

} // namespace mft
