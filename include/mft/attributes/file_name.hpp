#pragma once

#include "mft/attributes/attribute.hpp"
#include "mft/attributes/file_info.hpp"
#include <memory>

namespace mft {
namespace attributes {

/**
 * @brief FileName attribute ($FILE_NAME)
 * 
 * Contains file name and associated metadata.
 * A file can have multiple FileName attributes (DOS, Windows, etc.)
 */
class FileName : public Attribute {
public:
    /**
     * @brief Construct from raw bytes
     */
    FileName(const uint8_t* raw_bytes, size_t length);
    
    /**
     * @brief Get the file info
     */
    const FileInfo& file_info() const { return file_info_; }
    
    std::string to_string() const override;

private:
    FileInfo file_info_;
};

} // namespace attributes
} // namespace mft
