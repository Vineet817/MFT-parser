#pragma once

#include <cstdint>
#include <string>
#include <istream>

namespace boot {

/**
 * @brief NTFS Boot Sector Parser
 * 
 * Parses the $Boot file (first sector of NTFS volume) to extract
 * volume metadata like bytes per sector, clusters, MFT location, etc.
 */
class Boot {
public:
    static constexpr uint16_t EXPECTED_SECTOR_SIG = 0xAA55;
    
    /**
     * @brief Construct from input stream
     * @param file_stream Stream containing boot sector data (512 bytes minimum)
     */
    explicit Boot(std::istream& file_stream);
    
    // BPB (BIOS Parameter Block) fields
    std::string boot_entry_point() const { return boot_entry_point_; }
    std::string file_system_signature() const { return file_system_signature_; }
    
    int32_t bytes_per_sector() const { return bytes_per_sector_; }
    int32_t sector_signature() const { return sector_signature_; }
    int32_t sectors_per_cluster() const { return sectors_per_cluster_; }
    
    // Not used by NTFS but present in BPB
    int32_t reserved_sectors() const { return reserved_sectors_; }
    int32_t number_of_fats() const { return number_of_fats_; }
    int32_t root_directory_entries() const { return root_directory_entries_; }
    int32_t total_number_of_sectors16() const { return total_number_of_sectors16_; }
    uint8_t media_descriptor() const { return media_descriptor_; }
    int32_t sectors_per_fat() const { return sectors_per_fat_; }
    int32_t sectors_per_track() const { return sectors_per_track_; }
    int32_t number_of_heads() const { return number_of_heads_; }
    int32_t number_of_hidden_sectors() const { return number_of_hidden_sectors_; }
    int32_t total_number_of_sectors() const { return total_number_of_sectors_; }
    
    // NTFS-specific fields
    uint8_t disk_unit_number() const { return disk_unit_number_; }
    uint8_t unknown_flags() const { return unknown_flags_; }
    uint8_t bpb_version_signature() const { return bpb_version_signature_; }
    uint8_t unknown_reserved() const { return unknown_reserved_; }
    
    int64_t total_sectors() const { return total_sectors_; }
    int64_t mft_cluster_block_number() const { return mft_cluster_block_number_; }
    int64_t mirror_mft_cluster_block_number() const { return mirror_mft_cluster_block_number_; }
    
    int32_t mft_entry_size() const { return mft_entry_size_; }
    int32_t index_entry_size() const { return index_entry_size_; }
    
    int64_t volume_serial_number_raw() const { return volume_serial_number_raw_; }
    int32_t checksum() const { return checksum_; }
    
    // Helper methods
    std::string decode_media_descriptor() const;
    std::string get_sector_signature() const;
    std::string get_volume_serial_number(bool as_32bit = false, bool reverse = false) const;

private:
    std::string boot_entry_point_;
    std::string file_system_signature_;
    
    int32_t bytes_per_sector_;
    int32_t sector_signature_;
    int32_t sectors_per_cluster_;
    int32_t reserved_sectors_;
    int32_t number_of_fats_;
    int32_t root_directory_entries_;
    int32_t total_number_of_sectors16_;
    uint8_t media_descriptor_;
    int32_t sectors_per_fat_;
    int32_t sectors_per_track_;
    int32_t number_of_heads_;
    int32_t number_of_hidden_sectors_;
    int32_t total_number_of_sectors_;
    
    uint8_t disk_unit_number_;
    uint8_t unknown_flags_;
    uint8_t bpb_version_signature_;
    uint8_t unknown_reserved_;
    
    int64_t total_sectors_;
    int64_t mft_cluster_block_number_;
    int64_t mirror_mft_cluster_block_number_;
    
    int32_t mft_entry_size_;
    int32_t index_entry_size_;
    
    int64_t volume_serial_number_raw_;
    int32_t checksum_;
    
    static int32_t get_size_as_bytes(uint8_t size, int32_t cluster_size);
};

/**
 * @brief Static helper for loading boot files
 */
class BootFile {
public:
    static const std::string DATE_TIME_FORMAT;
    
    /**
     * @brief Load and parse a $Boot file
     */
    static std::unique_ptr<Boot> load(const std::string& boot_path);
};

} // namespace boot
