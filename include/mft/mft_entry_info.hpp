#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace mft {

/**
 * @brief Represents an MFT entry reference (8 bytes)
 * 
 * Contains entry number (6 bytes) and sequence number (2 bytes).
 * Used to reference other MFT records.
 */
class MftEntryInfo {
public:
    /**
     * @brief Construct from 8-byte raw data
     * @param raw_bytes Pointer to 8 bytes of data
     */
    explicit MftEntryInfo(const uint8_t* raw_bytes);
    
    /**
     * @brief Default constructor (creates empty reference)
     */
    MftEntryInfo() : mft_entry_number_(0), mft_sequence_number_(0) {}
    
    /**
     * @brief Get the MFT entry number
     */
    uint32_t mft_entry_number() const { return mft_entry_number_; }
    
    /**
     * @brief Get the MFT sequence number
     */
    uint16_t mft_sequence_number() const { return mft_sequence_number_; }
    
    /**
     * @brief Set the MFT entry number
     */
    void set_mft_entry_number(uint32_t value) { mft_entry_number_ = value; }
    
    /**
     * @brief Set the MFT sequence number
     */
    void set_mft_sequence_number(uint16_t value) { mft_sequence_number_ = value; }
    
    /**
     * @brief Get a key string for map lookups
     * @param as_decimal If true, use decimal format; otherwise hex
     * @return Key string like "0000001F-00000001" or "31-1"
     */
    std::string get_key(bool as_decimal = false) const;
    
    /**
     * @brief Convert to string representation
     */
    std::string to_string() const;
    
    /**
     * @brief Check if this is a null/empty reference
     */
    bool is_empty() const { 
        return mft_entry_number_ == 0 && mft_sequence_number_ == 0; 
    }
    
    bool operator==(const MftEntryInfo& other) const {
        return mft_entry_number_ == other.mft_entry_number_ && 
               mft_sequence_number_ == other.mft_sequence_number_;
    }
    
    bool operator!=(const MftEntryInfo& other) const {
        return !(*this == other);
    }

private:
    uint32_t mft_entry_number_;
    uint16_t mft_sequence_number_;
};

} // namespace mft
