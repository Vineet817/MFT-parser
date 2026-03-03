#pragma once

#include <vector>
#include <string>
#include <memory>
#include <istream>
#include "mft/common/datetime_utils.hpp"

namespace usn {

/**
 * @brief USN Journal Record (v2)
 */
class UsnRecord {
public:
    UsnRecord(const uint8_t* raw_bytes, size_t length);
    
    uint32_t record_length() const { return record_length_; }
    uint16_t major_version() const { return major_version_; }
    uint16_t minor_version() const { return minor_version_; }
    uint64_t file_reference_number() const { return file_reference_number_; }
    uint64_t parent_file_reference_number() const { return parent_file_reference_number_; }
    uint64_t usn() const { return usn_; }
    const mft::common::NullableDateTime& timestamp() const { return timestamp_; }
    uint32_t reason() const { return reason_; }
    uint32_t source_info() const { return source_info_; }
    uint32_t security_id() const { return security_id_; }
    uint32_t file_attributes() const { return file_attributes_; }
    const std::string& filename() const { return filename_; }
    
    std::string to_string() const;

private:
    uint32_t record_length_;
    uint16_t major_version_;
    uint16_t minor_version_;
    uint64_t file_reference_number_;
    uint64_t parent_file_reference_number_;
    uint64_t usn_;
    mft::common::NullableDateTime timestamp_;
    uint32_t reason_;
    uint32_t source_info_;
    uint32_t security_id_;
    uint32_t file_attributes_;
    std::string filename_;
};

class UsnJournal {
public:
    explicit UsnJournal(std::istream& file_stream);
    
    const std::vector<UsnRecord>& records() const { return records_; }

private:
    std::vector<UsnRecord> records_;
};

class UsnFile {
public:
    static std::unique_ptr<UsnJournal> load(const std::string& usn_path);
};

} // namespace usn
