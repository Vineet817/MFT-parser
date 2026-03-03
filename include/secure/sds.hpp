#pragma once

#include <vector>
#include <string>
#include <memory>
#include <istream>

namespace secure {

/**
 * @brief $Secure/$SDS Parser
 * 
 * Parses the Security Descriptor Stream which contains all unique security descriptors
 * on the volume.
 */
class Sds {
public:
    explicit Sds(std::istream& file_stream);
    
    // TODO: Add methods to retrieve security descriptors by ID
    
private:
    // Future: Cache/Map of security descriptors
};

class SdsFile {
public:
    static std::unique_ptr<Sds> load(const std::string& sds_path);
};

} // namespace secure
