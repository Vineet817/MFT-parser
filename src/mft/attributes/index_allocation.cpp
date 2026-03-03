#include "mft/attributes/index_allocation.hpp"
#include <sstream>

namespace mft {
namespace attributes {

IndexAllocation::IndexAllocation(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length)
{
    // Index Allocation is almost always non-resident and contains INDX records
    // parsing happens via DataRuns like Data attribute
}

std::string IndexAllocation::to_string() const {
    std::ostringstream oss;
    oss << "**** INDEX ALLOCATION ****\n";
    oss << this->Attribute::to_string() << "\n";
    
    // Uses standard non-resident display from base class (via Attribute::to_string calling data runs display if we added it)
    // Actually base Attribute::to_string doesn't display run lists by default unless we add it specifically or cast
    // For now, just the header info is enough as content is massive
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
