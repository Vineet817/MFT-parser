#include "boot/boot.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <bitset>

namespace boot {

using mft::common::ByteReader;
using mft::common::Logger;

Boot::Boot(std::istream& file_stream)
    : bytes_per_sector_(0),
      sector_signature_(0),
      sectors_per_cluster_(0),
      reserved_sectors_(0),
      number_of_fats_(0),
      root_directory_entries_(0),
      total_number_of_sectors16_(0),
      media_descriptor_(0),
      sectors_per_fat_(0),
      sectors_per_track_(0),
      number_of_heads_(0),
      number_of_hidden_sectors_(0),
      total_number_of_sectors_(0),
      disk_unit_number_(0),
      unknown_flags_(0),
      bpb_version_signature_(0),
      unknown_reserved_(0),
      total_sectors_(0),
      mft_cluster_block_number_(0),
      mirror_mft_cluster_block_number_(0),
      mft_entry_size_(0),
      index_entry_size_(0),
      volume_serial_number_raw_(0),
      checksum_(0)
{
    uint8_t raw_bytes[512];
    file_stream.read(reinterpret_cast<char*>(raw_bytes), 512);
    
    sector_signature_ = ByteReader::read_uint16(raw_bytes, 510);
    
    if (sector_signature_ != EXPECTED_SECTOR_SIG) {
        Logger::warning_fmt(
            "Expected signature (0x55 0xAA) not found at offset 0x1FE. Value found: {}",
            get_sector_signature());
    }
    
    // Boot entry point (first 3 bytes)
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << std::setfill('0')
        << std::setw(2) << static_cast<int>(raw_bytes[0]) << " 0x"
        << std::setw(2) << static_cast<int>(raw_bytes[1]) << " 0x"
        << std::setw(2) << static_cast<int>(raw_bytes[2]);
    boot_entry_point_ = oss.str();
    
    // File system signature (8 bytes at offset 3)
    file_system_signature_ = ByteReader::read_ascii_string(raw_bytes, 3, 8);
    
    bytes_per_sector_ = ByteReader::read_int16(raw_bytes, 11);
    sectors_per_cluster_ = raw_bytes[13];
    
    reserved_sectors_ = ByteReader::read_int16(raw_bytes, 14);
    number_of_fats_ = raw_bytes[16];
    
    root_directory_entries_ = ByteReader::read_int16(raw_bytes, 17);
    total_number_of_sectors16_ = ByteReader::read_int16(raw_bytes, 19);
    
    media_descriptor_ = raw_bytes[21];
    
    sectors_per_fat_ = ByteReader::read_int16(raw_bytes, 22);
    sectors_per_track_ = ByteReader::read_int16(raw_bytes, 24);
    number_of_heads_ = ByteReader::read_int16(raw_bytes, 26);
    number_of_hidden_sectors_ = ByteReader::read_int32(raw_bytes, 28);
    total_number_of_sectors_ = ByteReader::read_int32(raw_bytes, 32);
    
    disk_unit_number_ = raw_bytes[36];
    unknown_flags_ = raw_bytes[37];
    bpb_version_signature_ = raw_bytes[38];
    unknown_reserved_ = raw_bytes[39];
    
    total_sectors_ = ByteReader::read_int64(raw_bytes, 40);
    mft_cluster_block_number_ = ByteReader::read_int64(raw_bytes, 48);
    mirror_mft_cluster_block_number_ = ByteReader::read_int64(raw_bytes, 56);
    
    int32_t cluster_size = bytes_per_sector_ * sectors_per_cluster_;
    
    uint8_t mft_entry_size_raw = raw_bytes[64];
    mft_entry_size_ = get_size_as_bytes(mft_entry_size_raw, cluster_size);
    
    uint8_t index_entry_size_raw = raw_bytes[68];
    index_entry_size_ = get_size_as_bytes(index_entry_size_raw, cluster_size);
    
    volume_serial_number_raw_ = ByteReader::read_int64(raw_bytes, 72);
    checksum_ = ByteReader::read_int32(raw_bytes, 80);
}

int32_t Boot::get_size_as_bytes(uint8_t size, int32_t cluster_size) {
    if (size <= 127) {
        return static_cast<int32_t>(size) * cluster_size;
    }
    return static_cast<int32_t>(std::pow(2, 256 - size));
}

std::string Boot::decode_media_descriptor() const {
    std::string desc;
    std::bitset<8> md_bits(media_descriptor_);
    
    // Bit 0: sides
    if (md_bits[0]) {
        desc += "Double-sided";
    } else {
        desc += "Single-sided";
    }
    
    // Bit 1: sectors per track
    if (md_bits[1]) {
        desc += ", 8 sectors per track";
    } else {
        desc += ", 9 sectors per track";
    }
    
    // Bit 2: tracks
    if (md_bits[2]) {
        desc += ", 40 tracks";
    } else {
        desc += ", 80 tracks";
    }
    
    // Bit 3: removable
    if (md_bits[3]) {
        desc += ", Removable disc";
    } else {
        desc += ", Fixed disc";
    }
    
    return desc;
}

std::string Boot::get_sector_signature() const {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0')
        << std::setw(2) << (sector_signature_ & 0xFF) << " "
        << std::setw(2) << ((sector_signature_ >> 8) & 0xFF);
    return oss.str();
}

std::string Boot::get_volume_serial_number(bool as_32bit, bool reverse) const {
    std::ostringstream oss;
    uint8_t bytes[8];
    std::memcpy(bytes, &volume_serial_number_raw_, 8);
    
    int count = as_32bit ? 4 : 8;
    
    if (reverse && as_32bit) {
        for (int i = 3; i >= 0; --i) {
            if (i < 3) oss << " ";
            oss << std::hex << std::uppercase << std::setfill('0')
                << std::setw(2) << static_cast<int>(bytes[i]);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            if (i > 0) oss << " ";
            oss << std::hex << std::uppercase << std::setfill('0')
                << std::setw(2) << static_cast<int>(bytes[i]);
        }
    }
    
    return oss.str();
}

// BootFile implementation
const std::string BootFile::DATE_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

std::unique_ptr<Boot> BootFile::load(const std::string& boot_path) {
    std::ifstream file(boot_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("'" + boot_path + "' not found");
    }
    return std::make_unique<Boot>(file);
}

} // namespace boot
