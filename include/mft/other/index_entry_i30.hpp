#pragma once

#include "mft/mft_entry_info.hpp"
#include "mft/attributes/file_info.hpp"
#include "mft/common/datetime_utils.hpp"
#include <string>

namespace mft {
namespace other {

/**
 * @brief I30 Index Entry
 * 
 * Represents an entry in the $I30 directory index.
 */
class IndexEntryI30 {
public:
    enum class OEntryFlag : int32_t {
        HasSubNodes = 0x1,
        LastEntry = 0x2
    };
    
    /**
     * @brief Construct from raw bytes
     * @param raw_bytes Raw index entry data
     * @param length Data length
     * @param absolute_offset Offset in the original file
     * @param page_number Page number in the index
     * @param from_slack Whether recovered from slack space
     */
    IndexEntryI30(const uint8_t* raw_bytes, size_t length, 
                  int64_t absolute_offset, int page_number, bool from_slack);
    
    OEntryFlag flag() const { return flag_; }
    int page_number() const { return page_number_; }
    bool from_slack() const { return from_slack_; }
    int64_t absolute_offset() const { return absolute_offset_; }
    
    const MftEntryInfo& mft_reference_self() const { return mft_reference_self_; }
    const attributes::FileInfo* file_info() const { return file_info_.get(); }
    
    const std::string& md5() const { return md5_; }
    void set_md5(const std::string& md5) { md5_ = md5; }
    
    std::string to_string() const;

private:
    OEntryFlag flag_;
    int page_number_;
    bool from_slack_;
    int64_t absolute_offset_;
    MftEntryInfo mft_reference_self_;
    std::unique_ptr<attributes::FileInfo> file_info_;
    std::string md5_;
};

} // namespace other
} // namespace mft
