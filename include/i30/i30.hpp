#pragma once

#include "mft/other/index_entry_i30.hpp"
#include <vector>
#include <memory>
#include <istream>

namespace i30 {

/**
 * @brief $I30 Index Parser
 * 
 * Parses the $I30 directory index file which contains
 * directory entries including deleted file references.
 */
class I30 {
public:
    /**
     * @brief Construct from input stream
     * @param file_stream Stream containing $I30 data
     */
    explicit I30(std::istream& file_stream);
    
    /**
     * @brief Get all parsed entries
     */
    const std::vector<mft::other::IndexEntryI30>& entries() const { 
        return entries_; 
    }

private:
    std::vector<mft::other::IndexEntryI30> entries_;
};

/**
 * @brief Static helper for loading I30 files
 */
class I30File {
public:
    static const std::string DATE_TIME_FORMAT;
    
    /**
     * @brief Load and parse an $I30 file
     */
    static std::unique_ptr<I30> load(const std::string& i30_path);
};

} // namespace i30
