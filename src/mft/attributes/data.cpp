#include "mft/attributes/data.hpp"
#include "mft/common/byte_reader.hpp"
#include <sstream>
#include <iomanip>

namespace mft {
namespace other {

std::string DataRun::to_string() const {
    std::ostringstream oss;
    oss << std::hex << std::uppercase
        << "Cluster Count: 0x" << cluster_count_
        << ", Cluster Offset: 0x" << cluster_offset_;
    return oss.str();
}

} // namespace other

namespace attributes {

// ResidentData implementation
ResidentData::ResidentData(const uint8_t* raw_bytes, size_t length) {
    data_.assign(raw_bytes, raw_bytes + length);
}

std::string ResidentData::to_string() const {
    std::ostringstream oss;
    oss << "Data length: 0x" << std::hex << data_.size();
    if (data_.size() <= 64) {
        oss << " Content: " << common::ByteReader::to_hex_string(data_.data(), data_.size());
    }
    return oss.str();
}

// NonResidentData implementation
NonResidentData::NonResidentData(const uint8_t* raw_bytes, size_t length)
    : starting_vcn_(0), ending_vcn_(0), allocated_size_(0), 
      actual_size_(0), initialized_size_(0)
{
    using common::ByteReader;
    
    starting_vcn_ = ByteReader::read_uint64(raw_bytes, 0x10);
    ending_vcn_ = ByteReader::read_uint64(raw_bytes, 0x18);
    
    uint16_t offset_to_data_runs = ByteReader::read_uint16(raw_bytes, 0x20);
    
    allocated_size_ = ByteReader::read_uint64(raw_bytes, 0x28);
    actual_size_ = ByteReader::read_uint64(raw_bytes, 0x30);
    initialized_size_ = ByteReader::read_uint64(raw_bytes, 0x38);
    
    // Parse data runs
    size_t index = offset_to_data_runs;
    uint8_t dr_start = raw_bytes[index];
    
    while (dr_start != 0 && index < length) {
        uint8_t offset_length = (dr_start & 0xF0) >> 4;  // left nibble
        uint8_t cluster_len_byte_count = dr_start & 0x0F;  // right nibble
        index += 1;
        
        // Read run length
        uint8_t run_len_bytes[8] = {0};
        for (uint8_t i = 0; i < cluster_len_byte_count && index < length; ++i) {
            run_len_bytes[i] = raw_bytes[index++];
        }
        uint64_t cluster_run_length = *reinterpret_cast<uint64_t*>(run_len_bytes);
        
        // Read cluster offset (can be negative for sparse)
        uint8_t cluster_bytes[8] = {0};
        for (uint8_t i = 0; i < offset_length && index < length; ++i) {
            cluster_bytes[i] = raw_bytes[index++];
        }
        
        // Handle negative offsets (sign extension)
        if (offset_length > 0 && cluster_bytes[offset_length - 1] >= 0x80) {
            for (size_t i = offset_length; i < 8; ++i) {
                cluster_bytes[i] = 0xFF;
            }
        }
        
        int64_t cluster_number = *reinterpret_cast<int64_t*>(cluster_bytes);
        
        data_runs_.emplace_back(cluster_run_length, cluster_number);
        
        if (index < length) {
            dr_start = raw_bytes[index];
        } else {
            break;
        }
    }
}

std::string NonResidentData::to_string() const {
    std::ostringstream oss;
    oss << std::hex << std::uppercase
        << "\nStarting Virtual Cluster #: 0x" << starting_vcn_
        << ", Ending Virtual Cluster #: 0x" << ending_vcn_
        << ", Allocated Size: 0x" << allocated_size_
        << ", Actual Size: 0x" << actual_size_
        << ", Initialized Size: 0x" << initialized_size_ << "\n\n"
        << "DataRuns Entries\n";
    
    for (const auto& dr : data_runs_) {
        oss << dr.to_string() << "\n";
    }
    
    return oss.str();
}

// Data implementation
Data::Data(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    if (is_resident_) {
        size_t content_len = static_cast<size_t>(attribute_content_length_);
        resident_data_ = std::make_unique<ResidentData>(
            raw_bytes + content_offset_, content_len);
    } else {
        non_resident_data_ = std::make_unique<NonResidentData>(raw_bytes, length);
    }
}

std::string Data::to_string() const {
    std::ostringstream oss;
    oss << "**** DATA ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    
    if (resident_data_) {
        oss << "Resident Data\n" << resident_data_->to_string();
    } else if (non_resident_data_) {
        oss << "Non Resident Data\n" << non_resident_data_->to_string();
    }
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
