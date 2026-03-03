#include "mft/attributes/reparse_point.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include <sstream>

namespace mft {
namespace attributes {

std::string reparse_point_tag_to_string(ReparsePointTag tag) {
    switch (tag) {
        case ReparsePointTag::ReservedZero: return "ReservedZero";
        case ReparsePointTag::ReservedOne: return "ReservedOne";
        case ReparsePointTag::ReservedTwo: return "ReservedTwo";
        case ReparsePointTag::DriverExtender: return "DriverExtender";
        case ReparsePointTag::HierarchicalStorageManager2: return "HierarchicalStorageManager2";
        case ReparsePointTag::SisFilterDriver: return "SisFilterDriver";
        case ReparsePointTag::Wim: return "Wim";
        case ReparsePointTag::Csv: return "Csv";
        case ReparsePointTag::DistributedFileSystem: return "DistributedFileSystem";
        case ReparsePointTag::FilterManagerTestHarness: return "FilterManagerTestHarness";
        case ReparsePointTag::DistributedFileSystemR: return "DistributedFileSystemR";
        case ReparsePointTag::DeDupe: return "DeDupe";
        case ReparsePointTag::Nfs: return "Nfs";
        case ReparsePointTag::FilePlaceHolder: return "FilePlaceHolder";
        case ReparsePointTag::Wof: return "Wof";
        case ReparsePointTag::Wci: return "Wci";
        case ReparsePointTag::GlobalReparse: return "GlobalReparse";
        case ReparsePointTag::AppExeCLink: return "AppExeCLink";
        case ReparsePointTag::Hfs: return "Hfs";
        case ReparsePointTag::Unhandled: return "Unhandled";
        case ReparsePointTag::OneDrive: return "OneDrive";
        case ReparsePointTag::Cloud: return "Cloud";
        case ReparsePointTag::CloudRoot: return "CloudRoot";
        case ReparsePointTag::CloudOnDemand: return "CloudOnDemand";
        case ReparsePointTag::CloudRootOnDemand: return "CloudRootOnDemand";
        case ReparsePointTag::Gvfs: return "Gvfs";
        case ReparsePointTag::MountPoint: return "MountPoint";
        case ReparsePointTag::SymbolicLink: return "SymbolicLink";
        case ReparsePointTag::IisCache: return "IisCache";
        case ReparsePointTag::LxSymLink: return "LxSymLink";
        case ReparsePointTag::WciTombstone: return "WciTombstone";
        case ReparsePointTag::GvfsTombstone: return "GvfsTombstone";
        case ReparsePointTag::HierarchicalStorageManager: return "HierarchicalStorageManager";
        case ReparsePointTag::AppXStrim: return "AppXStrim";
        default: return "Unknown";
    }
}

ReparsePoint::ReparsePoint(const uint8_t* raw_bytes, size_t length)
    : Attribute(raw_bytes, length),
      tag_(ReparsePointTag::ReservedZero),
      symlink_flag_relative_(false)
{
    using common::ByteReader;
    using common::Logger;
    
    if (attribute_content_length_ == 0 || attribute_content_length_ == 8) {
        return;
    }
    
    std::vector<uint8_t> content(attribute_content_length_);
    ByteReader::block_copy(raw_bytes, content_offset_, content.data(), 0, attribute_content_length_);
    
    uint32_t tag_val = ByteReader::read_uint32(content.data(), 0);
    
    // Map tag value to enum
    switch (tag_val) {
        case 0x00000000: tag_ = ReparsePointTag::ReservedZero; break;
        case 0x00000001: tag_ = ReparsePointTag::ReservedOne; break;
        case 0x00000002: tag_ = ReparsePointTag::ReservedTwo; break;
        case 0x80000005: tag_ = ReparsePointTag::DriverExtender; break;
        case 0x80000006: tag_ = ReparsePointTag::HierarchicalStorageManager2; break;
        case 0x80000007: tag_ = ReparsePointTag::SisFilterDriver; break;
        case 0x80000008: tag_ = ReparsePointTag::Wim; break;
        case 0x80000009: tag_ = ReparsePointTag::Csv; break;
        case 0x8000000a: tag_ = ReparsePointTag::DistributedFileSystem; break;
        case 0x8000000b: tag_ = ReparsePointTag::FilterManagerTestHarness; break;
        case 0x80000012: tag_ = ReparsePointTag::DistributedFileSystemR; break;
        case 0x80000013: tag_ = ReparsePointTag::DeDupe; break;
        case 0x80000014: tag_ = ReparsePointTag::Nfs; break;
        case 0x80000015: tag_ = ReparsePointTag::FilePlaceHolder; break;
        case 0x80000017: tag_ = ReparsePointTag::Wof; break;
        case 0x80000018: tag_ = ReparsePointTag::Wci; break;
        case 0x80000019: tag_ = ReparsePointTag::GlobalReparse; break;
        case 0x8000001B: tag_ = ReparsePointTag::AppExeCLink; break;
        case 0x8000001E: tag_ = ReparsePointTag::Hfs; break;
        case 0x80000020: tag_ = ReparsePointTag::Unhandled; break;
        case 0x80000021: tag_ = ReparsePointTag::OneDrive; break;
        case 0x9000001A: tag_ = ReparsePointTag::Cloud; break;
        case 0x9000101A: tag_ = ReparsePointTag::CloudRoot; break;
        case 0x9000201A: tag_ = ReparsePointTag::CloudOnDemand; break;
        case 0x9000301A: tag_ = ReparsePointTag::CloudRootOnDemand; break;
        case 0x9000001C: tag_ = ReparsePointTag::Gvfs; break;
        case 0xa0000003: tag_ = ReparsePointTag::MountPoint; break;
        case 0xa000000c: tag_ = ReparsePointTag::SymbolicLink; break;
        case 0xA0000010: tag_ = ReparsePointTag::IisCache; break;
        case 0xA000001D: tag_ = ReparsePointTag::LxSymLink; break;
        case 0xA000001F: tag_ = ReparsePointTag::WciTombstone; break;
        case 0xA0000022: tag_ = ReparsePointTag::GvfsTombstone; break;
        case 0xc0000004: tag_ = ReparsePointTag::HierarchicalStorageManager; break;
        case 0xC0000014: tag_ = ReparsePointTag::AppXStrim; break;
        default: tag_ = ReparsePointTag::Unhandled; break;
    }
    
    if (content.size() < 16) {
        return;
    }
    
    int16_t sub_name_offset = ByteReader::read_int16(content.data(), 8);
    int16_t sub_name_size = ByteReader::read_int16(content.data(), 10);
    
    if (sub_name_size == 0) {
        Logger::debug("SubstituteName length is 0! Determining PrintName");
        if (content.size() > 0x0c) {
            print_name_ = ByteReader::read_ascii_string(content.data(), 0x0c, content.size() - 0x0c);
        }
        return;
    }
    
    int16_t print_name_offset = ByteReader::read_int16(content.data(), 12);
    int16_t print_name_size = ByteReader::read_int16(content.data(), 14);
    
    if (tag_ != ReparsePointTag::SymbolicLink && tag_ != ReparsePointTag::MountPoint) {
        return;
    }
    
    if (tag_ == ReparsePointTag::SymbolicLink && content.size() >= 20) {
        int32_t sym_flags = ByteReader::read_int32(content.data(), 16);
        if (sym_flags == 0x1) {
            symlink_flag_relative_ = true;
        }
        sub_name_offset += 4;
    }
    
    if (sub_name_size > 0) {
        int adjusted_offset = (sub_name_offset == 0) ? 0x10 : 0x14;
        if (static_cast<size_t>(adjusted_offset + sub_name_size) <= content.size()) {
            substitute_name_ = ByteReader::read_unicode_string(content.data(), adjusted_offset, sub_name_size);
        }
    }
    
    if (print_name_size > 0) {
        int adjusted_print_offset;
        if (print_name_offset == 0) {
            adjusted_print_offset = (sub_name_offset == 0 ? 0x10 : 0x14) + sub_name_size;
        } else {
            adjusted_print_offset = (sub_name_offset == 0 ? 0x10 : 0x14) + print_name_offset;
        }
        if (static_cast<size_t>(adjusted_print_offset + print_name_size) <= content.size()) {
            print_name_ = ByteReader::read_unicode_string(content.data(), adjusted_print_offset, print_name_size);
        }
    }
}

std::string ReparsePoint::to_string() const {
    std::ostringstream oss;
    
    oss << "**** REPARSE POINT ****\n";
    oss << this->Attribute::to_string() << "\n\n";
    oss << "Substitute Name: " << substitute_name_
        << " Print Name: " << print_name_
        << " Tag: " << reparse_point_tag_to_string(tag_);
    
    if (symlink_flag_relative_) {
        oss << " (Relative)";
    }
    
    return oss.str();
}

} // namespace attributes
} // namespace mft
