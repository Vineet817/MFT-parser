#include "mft/mft_file.hpp"
#include <fstream>
#include <stdexcept>

namespace mft {

const std::string MftFile::DATE_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

std::unique_ptr<Mft> MftFile::load(const std::string& mft_path, bool recover_from_slack) {
    std::ifstream file(mft_path, std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("'" + mft_path + "' not found");
    }
    
    return std::make_unique<Mft>(file, recover_from_slack);
}

} // namespace mft
