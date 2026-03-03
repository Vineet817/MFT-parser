/**
 * MFT Parser CLI Tool
 * 
 * Parses NTFS structures ($MFT, $Boot, $I30) and outputs metadata to files.
 * Supports recovery of deleted files with resident data.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <chrono>

#include "mft/mft_file.hpp"
#include "mft/common/logger.hpp"
#include "mft/attributes/standard_info.hpp"
#include "mft/attributes/file_name.hpp"
#include "mft/attributes/data.hpp"
#include "mft/attributes/index_root.hpp"
#include "mft/attributes/reparse_point.hpp"
#include "mft/attributes/volume_name.hpp"
#include "mft/attributes/object_id.hpp"
#include "boot/boot.hpp"
#include "i30/i30.hpp"
#include "usn/usn.hpp"
#include "secure/sds.hpp"
#include "logfile/logfile.hpp"

namespace fs = std::filesystem;

void print_usage(const char* program_name) {
    std::cout << "MFT Parser CLI - NTFS Forensic Tool\n\n";
    std::cout << "Usage: " << program_name << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  mft <mft_file> <output_dir>       Parse $MFT and output metadata\n";
    std::cout << "  boot <boot_file> <output_dir>     Parse $Boot sector\n";
    std::cout << "  i30 <i30_file> <output_dir>       Parse $I30 directory index\n";
    std::cout << "  all <ntfs_dir> <output_dir>       Parse all available NTFS files\n";
    std::cout << "  recover <mft_file> <output_dir>   Recover deleted files\n\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --verbose    Enable verbose output\n";
    std::cout << "  -h, --help       Show this help message\n";
}

std::string format_timestamp(const mft::common::NullableDateTime& dt) {
    if (!dt.has_value()) {
        return "";
    }
    return dt.to_string();
}

std::string escape_csv(const std::string& str) {
    if (str.find(',') != std::string::npos || 
        str.find('"') != std::string::npos ||
        str.find('\n') != std::string::npos) {
        std::string escaped = "\"";
        for (char c : str) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    }
    return str;
}

// Sanitize filename for filesystem
std::string sanitize_filename(const std::string& name) {
    std::string result;
    for (char c : name) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || 
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            result += '_';
        } else {
            result += c;
        }
    }
    return result;
}

void export_mft_csv(const mft::Mft& mft, const fs::path& output_dir) {
    std::ofstream csv(output_dir / "mft_records.csv");
    
    // CSV Header
    csv << "EntryNumber,SequenceNumber,ParentEntry,ParentSeq,FileName,Extension,"
        << "IsDirectory,IsDeleted,FileSize,CreatedOn,ModifiedOn,RecordModifiedOn,"
        << "AccessedOn,Flags,ReparseTag,FullPath\n";
    
    auto export_records = [&](const auto& records, bool is_deleted) {
        for (const auto& [key, record] : records) {
            // Skip extension records
            if (record->mft_record_to_base_record().mft_entry_number() > 0 &&
                record->mft_record_to_base_record().mft_sequence_number() > 0) {
                continue;
            }
            
            // Find relevant attributes
            const mft::attributes::FileName* fn_attr = nullptr;
            const mft::attributes::StandardInfo* si_attr = nullptr;
            const mft::attributes::ReparsePoint* rp_attr = nullptr;
            
            for (const auto& attr : record->attributes()) {
                switch (attr->attribute_type()) {
                    case mft::attributes::AttributeType::FileName: {
                        auto* fn = dynamic_cast<const mft::attributes::FileName*>(attr.get());
                        // Prefer Windows name over DOS name
                        if (fn && (!fn_attr || fn->file_info().name_type() != mft::attributes::NameTypes::Dos)) {
                            fn_attr = fn;
                        }
                        break;
                    }
                    case mft::attributes::AttributeType::StandardInformation:
                        si_attr = dynamic_cast<const mft::attributes::StandardInfo*>(attr.get());
                        break;
                    case mft::attributes::AttributeType::ReparsePoint:
                        rp_attr = dynamic_cast<const mft::attributes::ReparsePoint*>(attr.get());
                        break;
                    default:
                        break;
                }
            }
            
            csv << record->entry_number() << ","
                << record->sequence_number() << ",";
            
            if (fn_attr) {
                csv << fn_attr->file_info().parent_mft_record().mft_entry_number() << ","
                    << fn_attr->file_info().parent_mft_record().mft_sequence_number() << ","
                    << escape_csv(fn_attr->file_info().file_name()) << ",";
                
                // Extract extension
                const auto& name = fn_attr->file_info().file_name();
                size_t dot_pos = name.rfind('.');
                if (dot_pos != std::string::npos && dot_pos != 0) {
                    csv << escape_csv(name.substr(dot_pos + 1));
                }
                csv << ",";
            } else {
                csv << ",,,,";
            }
            
            csv << (record->is_directory() ? "1" : "0") << ","
                << (is_deleted ? "1" : "0") << ",";
            
            // File size
            if (fn_attr) {
                csv << fn_attr->file_info().logical_size();
            }
            csv << ",";
            
            // Timestamps from StandardInfo
            if (si_attr) {
                csv << format_timestamp(si_attr->created_on()) << ","
                    << format_timestamp(si_attr->content_modified_on()) << ","
                    << format_timestamp(si_attr->record_modified_on()) << ","
                    << format_timestamp(si_attr->last_accessed_on()) << ","
                    << static_cast<int32_t>(si_attr->flags()) << ",";
            } else {
                csv << ",,,,,";
            }
            
            // Reparse tag
            if (rp_attr) {
                csv << mft::attributes::reparse_point_tag_to_string(rp_attr->tag());
            }
            csv << ",";
            
            // Full path
            if (fn_attr) {
                std::string parent_key = fn_attr->file_info().parent_mft_record().get_key();
                std::string full_path = mft.get_full_parent_path(parent_key);
                if (!full_path.empty()) {
                    full_path += "\\";
                }
                full_path += fn_attr->file_info().file_name();
                csv << escape_csv(full_path);
            }
            
            csv << "\n";
        }
    };
    
    export_records(mft.file_records(), false);
    export_records(mft.free_file_records(), true);
    
    csv.close();
    std::cout << "  Exported: mft_records.csv\n";
}

void export_mft_json_summary(const mft::Mft& mft, const fs::path& output_dir) {
    std::ofstream json(output_dir / "mft_summary.json");
    
    json << "{\n";
    json << "  \"file_size\": " << mft.file_size() << ",\n";
    json << "  \"active_records\": " << mft.file_records().size() << ",\n";
    json << "  \"deleted_records\": " << mft.free_file_records().size() << ",\n";
    json << "  \"bad_records\": " << mft.bad_records().size() << ",\n";
    json << "  \"uninitialized_records\": " << mft.uninitialized_records().size() << ",\n";
    
    // Count directories vs files
    size_t dir_count = 0, file_count = 0, deleted_dir_count = 0, deleted_file_count = 0;
    
    for (const auto& [key, record] : mft.file_records()) {
        if (record->is_directory()) dir_count++;
        else file_count++;
    }
    
    for (const auto& [key, record] : mft.free_file_records()) {
        if (record->is_directory()) deleted_dir_count++;
        else deleted_file_count++;
    }
    
    json << "  \"active_directories\": " << dir_count << ",\n";
    json << "  \"active_files\": " << file_count << ",\n";
    json << "  \"deleted_directories\": " << deleted_dir_count << ",\n";
    json << "  \"deleted_files\": " << deleted_file_count << "\n";
    json << "}\n";
    
    json.close();
    std::cout << "  Exported: mft_summary.json\n";
}

void export_mft_details(const mft::Mft& mft, const fs::path& output_dir) {
    fs::path details_dir = output_dir / "details";
    fs::create_directories(details_dir);
    
    // Export first 100 records with full attribute details
    int count = 0;
    for (const auto& [key, record] : mft.file_records()) {
        if (count++ >= 100) break;
        
        std::ofstream file(details_dir / ("record_" + std::to_string(record->entry_number()) + ".txt"));
        file << record->to_string();
        file.close();
    }
    
    std::cout << "  Exported: " << count << " detailed record files to details/\n";
}

// Recover deleted files
int recover_deleted_files(const std::string& mft_path, const std::string& output_path, bool verbose) {
    std::cout << "Recovering deleted files from: " << mft_path << "\n";
    
    fs::path output_dir(output_path);
    fs::path recovered_dir = output_dir / "recovered_files";
    fs::path resident_dir = recovered_dir / "resident";
    fs::path non_resident_dir = recovered_dir / "non_resident_info";
    
    fs::create_directories(resident_dir);
    fs::create_directories(non_resident_dir);
    
    try {
        if (verbose) {
            mft::common::Logger::set_level(mft::common::LogLevel::Debug);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        auto mft = mft::MftFile::load(mft_path, false);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "\nParsing completed in " << duration.count() << "ms\n";
        std::cout << "Deleted records found: " << mft->free_file_records().size() << "\n\n";
        
        // Summary CSV for all deleted files
        std::ofstream summary_csv(output_dir / "deleted_files_summary.csv");
        summary_csv << "EntryNumber,FileName,Extension,FileSize,DataType,Recoverable,"
                   << "RecoveredPath,CreatedOn,ModifiedOn,OriginalPath\n";
        
        int resident_recovered = 0;
        int non_resident_found = 0;
        int directories_found = 0;
        int no_data_attr = 0;
        
        for (const auto& [key, record] : mft->free_file_records()) {
            // Skip extension records
            if (record->mft_record_to_base_record().mft_entry_number() > 0 &&
                record->mft_record_to_base_record().mft_sequence_number() > 0) {
                continue;
            }
            
            // Find FileName attribute
            const mft::attributes::FileName* fn_attr = nullptr;
            const mft::attributes::StandardInfo* si_attr = nullptr;
            const mft::attributes::Data* data_attr = nullptr;
            
            for (const auto& attr : record->attributes()) {
                switch (attr->attribute_type()) {
                    case mft::attributes::AttributeType::FileName: {
                        auto* fn = dynamic_cast<const mft::attributes::FileName*>(attr.get());
                        if (fn && (!fn_attr || fn->file_info().name_type() != mft::attributes::NameTypes::Dos)) {
                            fn_attr = fn;
                        }
                        break;
                    }
                    case mft::attributes::AttributeType::StandardInformation:
                        si_attr = dynamic_cast<const mft::attributes::StandardInfo*>(attr.get());
                        break;
                    case mft::attributes::AttributeType::Data:
                        data_attr = dynamic_cast<const mft::attributes::Data*>(attr.get());
                        break;
                    default:
                        break;
                }
            }
            
            if (!fn_attr) continue;
            
            std::string file_name = fn_attr->file_info().file_name();
            std::string safe_name = sanitize_filename(file_name);
            
            // Get extension
            std::string extension;
            size_t dot_pos = file_name.rfind('.');
            if (dot_pos != std::string::npos && dot_pos != 0) {
                extension = file_name.substr(dot_pos + 1);
            }
            
            // Get original path
            std::string parent_key = fn_attr->file_info().parent_mft_record().get_key();
            std::string original_path = mft->get_full_parent_path(parent_key);
            if (!original_path.empty()) {
                original_path += "\\";
            }
            original_path += file_name;
            
            // Check if directory
            if (record->is_directory()) {
                directories_found++;
                summary_csv << record->entry_number() << ","
                           << escape_csv(file_name) << ","
                           << escape_csv(extension) << ","
                           << "0,Directory,No,,";
                if (si_attr) {
                    summary_csv << format_timestamp(si_attr->created_on()) << ","
                               << format_timestamp(si_attr->content_modified_on()) << ",";
                } else {
                    summary_csv << ",,";
                }
                summary_csv << escape_csv(original_path) << "\n";
                continue;
            }
            
            // No data attribute
            if (!data_attr) {
                no_data_attr++;
                summary_csv << record->entry_number() << ","
                           << escape_csv(file_name) << ","
                           << escape_csv(extension) << ","
                           << fn_attr->file_info().logical_size() << ",NoData,No,,";
                if (si_attr) {
                    summary_csv << format_timestamp(si_attr->created_on()) << ","
                               << format_timestamp(si_attr->content_modified_on()) << ",";
                } else {
                    summary_csv << ",,";
                }
                summary_csv << escape_csv(original_path) << "\n";
                continue;
            }
            
            // Check if resident (data stored in MFT record)
            if (data_attr->resident_data()) {
                const auto& data = data_attr->resident_data()->data();
                
                if (data.size() > 0) {
                    // Create unique filename
                    std::string output_name = std::to_string(record->entry_number()) + "_" + safe_name;
                    fs::path output_file = resident_dir / output_name;
                    
                    // Write recovered data
                    std::ofstream out(output_file, std::ios::binary);
                    out.write(reinterpret_cast<const char*>(data.data()), data.size());
                    out.close();
                    
                    resident_recovered++;
                    
                    summary_csv << record->entry_number() << ","
                               << escape_csv(file_name) << ","
                               << escape_csv(extension) << ","
                               << data.size() << ",Resident,Yes,"
                               << escape_csv(output_file.string()) << ",";
                    if (si_attr) {
                        summary_csv << format_timestamp(si_attr->created_on()) << ","
                                   << format_timestamp(si_attr->content_modified_on()) << ",";
                    } else {
                        summary_csv << ",,";
                    }
                    summary_csv << escape_csv(original_path) << "\n";
                    
                    if (verbose) {
                        std::cout << "  Recovered (resident): " << file_name 
                                 << " (" << data.size() << " bytes)\n";
                    }
                }
            } 
            // Non-resident - export data run info for manual recovery
            else if (data_attr->non_resident_data()) {
                const auto* nr_data = data_attr->non_resident_data();
                
                // Create info file with data runs
                std::string info_name = std::to_string(record->entry_number()) + "_" + safe_name + ".info";
                fs::path info_file = non_resident_dir / info_name;
                
                std::ofstream info(info_file);
                info << "File: " << file_name << "\n";
                info << "Entry Number: " << record->entry_number() << "\n";
                info << "Original Path: " << original_path << "\n";
                info << "Actual Size: " << nr_data->actual_size() << " bytes\n";
                info << "Allocated Size: " << nr_data->allocated_size() << " bytes\n";
                info << "Starting VCN: " << nr_data->starting_vcn() << "\n";
                info << "Ending VCN: " << nr_data->ending_vcn() << "\n\n";
                info << "Data Runs (cluster_count, cluster_offset):\n";
                info << "============================================\n";
                
                int64_t current_lcn = 0;
                for (const auto& run : nr_data->data_runs()) {
                    current_lcn += run.cluster_offset();
                    info << "  Count: " << run.cluster_count() 
                        << ", LCN: " << current_lcn << "\n";
                }
                
                info << "\nTo recover this file, you need:\n";
                info << "1. The raw disk image or volume\n";
                info << "2. The cluster size from $Boot (usually 4096 bytes)\n";
                info << "3. Read clusters from LCN positions above\n";
                info.close();
                
                non_resident_found++;
                
                summary_csv << record->entry_number() << ","
                           << escape_csv(file_name) << ","
                           << escape_csv(extension) << ","
                           << nr_data->actual_size() << ",NonResident,Partial,"
                           << escape_csv(info_file.string()) << ",";
                if (si_attr) {
                    summary_csv << format_timestamp(si_attr->created_on()) << ","
                               << format_timestamp(si_attr->content_modified_on()) << ",";
                } else {
                    summary_csv << ",,";
                }
                summary_csv << escape_csv(original_path) << "\n";
            }
        }
        
        summary_csv.close();
        
        std::cout << "\n=== Recovery Summary ===\n";
        std::cout << "Resident files recovered:    " << resident_recovered << " (fully recovered)\n";
        std::cout << "Non-resident files found:    " << non_resident_found << " (info exported)\n";
        std::cout << "Deleted directories:         " << directories_found << "\n";
        std::cout << "Files without data attr:     " << no_data_attr << "\n";
        std::cout << "\nOutput:\n";
        std::cout << "  Summary CSV:       " << (output_dir / "deleted_files_summary.csv") << "\n";
        std::cout << "  Recovered files:   " << resident_dir << "\n";
        std::cout << "  Non-resident info: " << non_resident_dir << "\n";
        
        std::cout << "\nDone!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int parse_mft(const std::string& mft_path, const std::string& output_path, bool verbose) {
    std::cout << "Parsing MFT: " << mft_path << "\n";
    
    fs::path output_dir(output_path);
    fs::create_directories(output_dir);
    
    try {
        if (verbose) {
            mft::common::Logger::set_level(mft::common::LogLevel::Debug);
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto mft = mft::MftFile::load(mft_path, false);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "\nParsing completed in " << duration.count() << "ms\n";
        std::cout << "  Active records: " << mft->file_records().size() << "\n";
        std::cout << "  Deleted records: " << mft->free_file_records().size() << "\n";
        std::cout << "  Bad records: " << mft->bad_records().size() << "\n";
        std::cout << "\nExporting to: " << output_dir << "\n";
        
        export_mft_csv(*mft, output_dir);
        export_mft_json_summary(*mft, output_dir);
        export_mft_details(*mft, output_dir);
        
        std::cout << "\nDone!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int parse_boot(const std::string& boot_path, const std::string& output_path, bool verbose) {
    std::cout << "Parsing Boot sector: " << boot_path << "\n";
    
    fs::path output_dir(output_path);
    fs::create_directories(output_dir);
    
    try {
        if (verbose) {
            mft::common::Logger::set_level(mft::common::LogLevel::Debug);
        }
        
        auto boot = boot::BootFile::load(boot_path);
        
        std::ofstream json(output_dir / "boot_info.json");
        
        json << "{\n";
        json << "  \"file_system\": \"" << boot->file_system_signature() << "\",\n";
        json << "  \"bytes_per_sector\": " << boot->bytes_per_sector() << ",\n";
        json << "  \"sectors_per_cluster\": " << boot->sectors_per_cluster() << ",\n";
        json << "  \"cluster_size\": " << (boot->bytes_per_sector() * boot->sectors_per_cluster()) << ",\n";
        json << "  \"total_sectors\": " << boot->total_sectors() << ",\n";
        json << "  \"mft_cluster_number\": " << boot->mft_cluster_block_number() << ",\n";
        json << "  \"mft_mirror_cluster_number\": " << boot->mirror_mft_cluster_block_number() << ",\n";
        json << "  \"mft_entry_size\": " << boot->mft_entry_size() << ",\n";
        json << "  \"index_entry_size\": " << boot->index_entry_size() << ",\n";
        json << "  \"volume_serial_number\": \"" << boot->get_volume_serial_number() << "\",\n";
        json << "  \"media_descriptor\": \"" << boot->decode_media_descriptor() << "\"\n";
        json << "}\n";
        
        json.close();
        
        std::cout << "\nBoot sector info exported to: " << output_dir / "boot_info.json" << "\n";
        std::cout << "\nDone!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int parse_i30(const std::string& i30_path, const std::string& output_path, bool verbose) {
    std::cout << "Parsing I30 index: " << i30_path << "\n";
    
    fs::path output_dir(output_path);
    fs::create_directories(output_dir);
    
    try {
        if (verbose) {
            mft::common::Logger::set_level(mft::common::LogLevel::Debug);
        }
        
        auto i30 = i30::I30File::load(i30_path);
        
        std::ofstream csv(output_dir / "i30_entries.csv");
        
        csv << "EntryNumber,SequenceNumber,ParentEntry,ParentSeq,FileName,"
            << "IsSlack,CreatedOn,ModifiedOn,RecordModifiedOn,AccessedOn,FileSize\n";
        
        for (const auto& entry : i30->entries()) {
            if (!entry.file_info()) continue;
            
            const auto& fi = *entry.file_info();
            
            csv << entry.mft_reference_self().mft_entry_number() << ","
                << entry.mft_reference_self().mft_sequence_number() << ","
                << fi.parent_mft_record().mft_entry_number() << ","
                << fi.parent_mft_record().mft_sequence_number() << ","
                << escape_csv(fi.file_name()) << ","
                << (entry.from_slack() ? "1" : "0") << ","
                << format_timestamp(fi.created_on()) << ","
                << format_timestamp(fi.content_modified_on()) << ","
                << format_timestamp(fi.record_modified_on()) << ","
                << format_timestamp(fi.last_accessed_on()) << ","
                << fi.logical_size() << "\n";
        }
        
        csv.close();
        
        std::cout << "\nI30 entries: " << i30->entries().size() << "\n";
        std::cout << "Exported to: " << output_dir / "i30_entries.csv" << "\n";
        std::cout << "\nDone!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

void find_and_parse_files(const fs::path& dir, const fs::path& output_base, bool verbose, int& result) {
    try {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            return;
        }

        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;

            std::string filename = entry.path().filename().string();
            std::string filepath = entry.path().string();
            
            // Check for $MFT
            if (filename == "$MFT" || filename == "MFT") {
                std::cout << "\n=== Found MFT at " << filepath << " ===\n";
                fs::path out = output_base / "mft";
                result |= parse_mft(filepath, out.string(), verbose);
            }
            // Check for $Boot
            else if (filename == "$Boot" || filename == "Boot") {
                std::cout << "\n=== Found Boot sector at " << filepath << " ===\n";
                fs::path out = output_base / "boot";
                result |= parse_boot(filepath, out.string(), verbose);
            }
            // Check for $I30 or files ending in $I30
            else if (filename == "$I30" || filename.find("$I30") != std::string::npos) {
                std::cout << "\n=== Found I30 index at " << filepath << " ===\n";
                fs::path out = output_base / "i30" / filename;
                result |= parse_i30(filepath, out.string(), verbose);
            }
            // Report on other interesting files
            else if (filename == "$J" || filename == "$UsnJrnl") {
                std::cout << "\n=== Found USN Journal at " << filepath << " ===\n";
                usn::UsnFile::load(filepath);
            }
            else if (filename == "$LogFile") {
                std::cout << "\n=== Found LogFile at " << filepath << " ===\n";
                logfile::LogFileParser::load(filepath);
            }
            else if (filename == "$Secure" || filename == "$SDS" || filename == "$Secure_$SDS") {
                std::cout << "\n=== Found Security Descriptor Stream at " << filepath << " ===\n";
                secure::SdsFile::load(filepath);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory " << dir << ": " << e.what() << "\n";
    }
}

int parse_all(const std::string& ntfs_dir, const std::string& output_path, bool verbose) {
    fs::path ntfs_path(ntfs_dir);
    fs::path output_dir(output_path);
    fs::create_directories(output_dir);
    
    std::cout << "Scanning for NTFS files in: " << ntfs_path << "\n";
    
    int result = 0;
    find_and_parse_files(ntfs_path, output_dir, verbose, result);
    
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    bool verbose = false;
    
    // Check for help flag
    if (command == "-h" || command == "--help") {
        print_usage(argv[0]);
        return 0;
    }
    
    // Check for verbose flag in remaining args
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
    }
    
    if (command == "mft") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " mft <mft_file> <output_dir>\n";
            return 1;
        }
        return parse_mft(argv[2], argv[3], verbose);
        
    } else if (command == "boot") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " boot <boot_file> <output_dir>\n";
            return 1;
        }
        return parse_boot(argv[2], argv[3], verbose);
        
    } else if (command == "i30") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " i30 <i30_file> <output_dir>\n";
            return 1;
        }
        return parse_i30(argv[2], argv[3], verbose);
        
    } else if (command == "all") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " all <ntfs_dir> <output_dir>\n";
            return 1;
        }
        return parse_all(argv[2], argv[3], verbose);
        
    } else if (command == "recover") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " recover <mft_file> <output_dir>\n";
            return 1;
        }
        return recover_deleted_files(argv[2], argv[3], verbose);
        
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage(argv[0]);
        return 1;
    }
}
