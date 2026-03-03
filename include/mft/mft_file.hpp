#pragma once

#include "mft/mft.hpp"
#include <string>
#include <memory>

namespace mft {

/**
 * @brief Static helper class for loading MFT files
 */
class MftFile {
public:
    /**
     * @brief Default datetime format string
     */
    static const std::string DATE_TIME_FORMAT;
    
    /**
     * @brief Load and parse an MFT file
     * @param mft_path Path to the $MFT file
     * @param recover_from_slack Whether to attempt slack space recovery
     * @return Parsed Mft object
     * @throws std::runtime_error if file not found or parse error
     */
    static std::unique_ptr<Mft> load(const std::string& mft_path, bool recover_from_slack);
};

} // namespace mft
