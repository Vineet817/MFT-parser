#include "mft/mft.hpp"
#include "mft/other/directory_name_map_value.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include "mft/attributes/file_name.hpp"
#include <algorithm>
#include <stack>

namespace mft {

int Mft::current_offset_ = 0;

Mft::Mft(std::istream& file_stream, bool recover_from_slack)
    : file_size_(0)
{
    using common::ByteReader;
    using common::Logger;
    
    // Get file size
    file_stream.seekg(0, std::ios::end);
    file_size_ = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);
    
    // Read first 4 bytes for signature check
    uint8_t header_bytes[4];
    file_stream.read(reinterpret_cast<char*>(header_bytes), 4);
    
    int32_t sig = ByteReader::read_int32(header_bytes, 0);
    if (sig != FileRecord::FILE_SIG) {
        throw std::runtime_error("Invalid header! Expected 'FILE' Signature.");
    }
    
    // Read block size from offset 0x1C
    file_stream.seekg(0x1C, std::ios::beg);
    uint8_t block_size_bytes[4];
    file_stream.read(reinterpret_cast<char*>(block_size_bytes), 4);
    int32_t block_size = ByteReader::read_int32(block_size_bytes, 0);
    
    file_stream.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> file_bytes(block_size);
    int index = 0;
    
    while (file_stream.good() && file_stream.tellg() < file_size_) {
        file_stream.read(reinterpret_cast<char*>(file_bytes.data()), block_size);
        
        if (file_stream.gcount() != block_size) {
            break;
        }
        
        current_offset_ = index;
        
        auto record = std::make_unique<FileRecord>(
            file_bytes.data(), block_size, index, recover_from_slack);
        
        std::string key = record->get_key();
        
        Logger::verbose_fmt("Offset: 0x{:X} flags: {} key: {}",
            record->offset(), entry_flag_to_string(record->entry_flags()), key);
        
        if (record->is_bad()) {
            bad_records_.push_back(std::move(record));
        } else if (record->is_uninitialized()) {
            uninitialized_records_.push_back(std::move(record));
        } else if (record->is_deleted()) {
            if (free_file_records_.find(key) != free_file_records_.end()) {
                Logger::warning_fmt(
                    "At offset 0x{:X}, a free FILE record with key '{}' already exists! Skipping...",
                    record->offset(), key);
            } else {
                free_file_records_[key] = std::move(record);
            }
        } else {
            if (file_records_.find(key) != file_records_.end()) {
                Logger::warning_fmt(
                    "At offset 0x{:X}, a FILE record with key '{}' already exists! Skipping...",
                    record->offset(), key);
            } else {
                // Check for extension records
                const auto& base_ref = record->mft_record_to_base_record();
                if (base_ref.mft_entry_number() > 0 && base_ref.mft_sequence_number() > 0) {
                    std::string base_key = base_ref.get_key();
                    extension_file_records_[base_key].push_back(record.get());
                }
                file_records_[key] = std::move(record);
            }
        }
        
        index += block_size;
    }
    
    current_offset_ = index;
    
    process_extension_blocks();
    build_maps(file_records_);
    build_maps(free_file_records_, false);
}

void Mft::process_extension_blocks() {
    using common::Logger;
    
    Logger::debug("Processing Extension FILE records");
    
    for (auto& [key, extensions] : extension_file_records_) {
        FileRecord* base_record = nullptr;
        
        auto it = file_records_.find(key);
        if (it != file_records_.end()) {
            base_record = it->second.get();
        } else {
            auto free_it = free_file_records_.find(key);
            if (free_it != free_file_records_.end()) {
                base_record = free_it->second.get();
            }
        }
        
        if (!base_record) {
            // Unassociated extension records
            unassociated_extension_records_[key] = extensions;
            continue;
        }
        
        Logger::debug_fmt("FILE record '{}', Extension records found: {}",
            key, extensions.size());
        
        // Pull in attributes from extension records
        for (auto* ext_record : extensions) {
            for (auto& attr : ext_record->attributes()) {
                // Clone the attribute (simplified - in real impl would need proper cloning)
                // For now, we'll just note that attributes would be merged
            }
        }
    }
}

void Mft::build_maps(const std::unordered_map<std::string, std::unique_ptr<FileRecord>>& records,
                    bool skip_unassociated)
{
    using common::Logger;
    using namespace attributes;
    
    for (const auto& [key, record] : records) {
        // Skip extension records
        if (record->mft_record_to_base_record().mft_entry_number() > 0 &&
            record->mft_record_to_base_record().mft_sequence_number() > 0) {
            continue;
        }
        
        if (record->attributes().empty()) {
            Logger::debug_fmt("Skipping file record at offset 0x{:X} has no attributes",
                record->offset());
            continue;
        }
        
        // Find FileName attributes
        for (const auto& attr : record->attributes()) {
            if (attr->attribute_type() != AttributeType::FileName) {
                continue;
            }
            
            const auto* fn_attr = dynamic_cast<const FileName*>(attr.get());
            if (!fn_attr) continue;
            
            const auto& file_info = fn_attr->file_info();
            
            // Skip DOS names
            if (file_info.name_type() == NameTypes::Dos) {
                continue;
            }
            
            // Add to directory map if this is a directory
            if (record->is_directory()) {
                std::string dir_key = record->get_key();
                if (directory_name_map_.find(dir_key) == directory_name_map_.end()) {
                    directory_name_map_[dir_key] = std::make_unique<other::DirectoryNameMapValue>(
                        file_info.file_name(),
                        file_info.parent_mft_record().get_key(),
                        record->is_deleted());
                }
            }
            
            // Add to parent map
            std::string parent_key = file_info.parent_mft_record().get_key();
            
            if (file_info.file_name() != ".") {
                other::ParentMapEntry entry(
                    file_info.file_name(),
                    record->get_key(true),
                    record->is_directory());
                parent_directory_name_map_[parent_key].insert(entry);
            }
        }
    }
    
    (void)skip_unassociated;  // TODO: Handle unassociated records
}

std::string Mft::get_full_parent_path(const std::string& record_key) const {
    std::stack<std::string> path_parts;
    std::string temp_key = record_key;
    
    while (directory_name_map_.find(temp_key) != directory_name_map_.end()) {
        const auto& dir = directory_name_map_.at(temp_key);
        path_parts.push(dir->name());
        
        if (temp_key == "00000005-00000005") {
            // Root directory
            break;
        }
        
        temp_key = dir->parent_record_key();
    }
    
    if (temp_key != "00000005-00000005") {
        path_parts.push(".\\PathUnknown\\Directory with ID 0x" + temp_key);
    }
    
    std::string result;
    while (!path_parts.empty()) {
        if (!result.empty()) {
            result += "\\";
        }
        result += path_parts.top();
        path_parts.pop();
    }
    
    return result;
}

std::vector<other::ParentMapEntry> Mft::get_directory_contents(const std::string& key) const {
    auto it = parent_directory_name_map_.find(key);
    if (it == parent_directory_name_map_.end()) {
        return {};
    }
    
    std::vector<other::ParentMapEntry> result(it->second.begin(), it->second.end());
    
    // Sort: directories first, then by name
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
            return a.is_directory() > b.is_directory();
        }
        return a.file_name() < b.file_name();
    });
    
    return result;
}

} // namespace mft
