#pragma once

#include "mft/attributes/attribute.hpp"
#include <string>

namespace mft {
namespace attributes {

/**
 * @brief Reparse point tags
 */
enum class ReparsePointTag : uint32_t {
    ReservedZero = 0x00000000,
    ReservedOne = 0x00000001,
    ReservedTwo = 0x00000002,
    DriverExtender = 0x80000005,
    HierarchicalStorageManager2 = 0x80000006,
    SisFilterDriver = 0x80000007,
    Wim = 0x80000008,
    Csv = 0x80000009,
    DistributedFileSystem = 0x8000000a,
    FilterManagerTestHarness = 0x8000000b,
    DistributedFileSystemR = 0x80000012,
    DeDupe = 0x80000013,
    Nfs = 0x80000014,
    FilePlaceHolder = 0x80000015,
    Wof = 0x80000017,
    Wci = 0x80000018,
    GlobalReparse = 0x80000019,
    AppExeCLink = 0x8000001B,
    Hfs = 0x8000001E,
    Unhandled = 0x80000020,
    OneDrive = 0x80000021,
    Cloud = 0x9000001A,
    CloudRoot = 0x9000101A,
    CloudOnDemand = 0x9000201A,
    CloudRootOnDemand = 0x9000301A,
    Gvfs = 0x9000001C,
    MountPoint = 0xa0000003,
    SymbolicLink = 0xa000000c,
    IisCache = 0xA0000010,
    LxSymLink = 0xA000001D,
    WciTombstone = 0xA000001F,
    GvfsTombstone = 0xA0000022,
    HierarchicalStorageManager = 0xc0000004,
    AppXStrim = 0xC0000014
};

std::string reparse_point_tag_to_string(ReparsePointTag tag);

/**
 * @brief ReparsePoint attribute ($REPARSE_POINT)
 * 
 * Contains reparse point data for mount points, symbolic links, etc.
 */
class ReparsePoint : public Attribute {
public:
    ReparsePoint(const uint8_t* raw_bytes, size_t length);
    
    const std::string& substitute_name() const { return substitute_name_; }
    const std::string& print_name() const { return print_name_; }
    ReparsePointTag tag() const { return tag_; }
    bool symlink_flag_relative() const { return symlink_flag_relative_; }
    
    std::string to_string() const override;

private:
    std::string substitute_name_;
    std::string print_name_;
    ReparsePointTag tag_;
    bool symlink_flag_relative_;
};

} // namespace attributes
} // namespace mft
