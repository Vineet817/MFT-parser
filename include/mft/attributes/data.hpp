#pragma once

#include "mft/attributes/attribute.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace mft {
namespace other {

/**
 * @brief Represents a data run (cluster extent)
 * 
 * Data runs describe where non-resident data is stored on disk.
 */
class DataRun {
public:
    DataRun(uint64_t cluster_count, int64_t cluster_offset)
        : cluster_count_(cluster_count), cluster_offset_(cluster_offset) {}
    
    uint64_t cluster_count() const { return cluster_count_; }
    int64_t cluster_offset() const { return cluster_offset_; }
    
    std::string to_string() const;

private:
    uint64_t cluster_count_;
    int64_t cluster_offset_;
};

} // namespace other

namespace attributes {

/**
 * @brief Resident data (stored within the MFT record)
 */
class ResidentData {
public:
    ResidentData(const uint8_t* raw_bytes, size_t length);
    
    const std::vector<uint8_t>& data() const { return data_; }
    
    std::string to_string() const;

private:
    std::vector<uint8_t> data_;
};

/**
 * @brief Non-resident data (stored in clusters on disk)
 */
class NonResidentData {
public:
    NonResidentData(const uint8_t* raw_bytes, size_t length);
    
    uint64_t starting_vcn() const { return starting_vcn_; }
    uint64_t ending_vcn() const { return ending_vcn_; }
    uint64_t allocated_size() const { return allocated_size_; }
    uint64_t actual_size() const { return actual_size_; }
    uint64_t initialized_size() const { return initialized_size_; }
    
    const std::vector<other::DataRun>& data_runs() const { return data_runs_; }
    
    std::string to_string() const;

private:
    uint64_t starting_vcn_;
    uint64_t ending_vcn_;
    uint64_t allocated_size_;
    uint64_t actual_size_;
    uint64_t initialized_size_;
    std::vector<other::DataRun> data_runs_;
};

/**
 * @brief Data attribute ($DATA)
 * 
 * Contains file content. Can be resident (small files) or non-resident (large files).
 */
class Data : public Attribute {
public:
    Data(const uint8_t* raw_bytes, size_t length);
    
    const ResidentData* resident_data() const { return resident_data_.get(); }
    const NonResidentData* non_resident_data() const { return non_resident_data_.get(); }
    
    std::string to_string() const override;

private:
    std::unique_ptr<ResidentData> resident_data_;
    std::unique_ptr<NonResidentData> non_resident_data_;
};

} // namespace attributes
} // namespace mft
