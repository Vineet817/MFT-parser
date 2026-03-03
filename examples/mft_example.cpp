/**
 * MFT Parser Example
 * 
 * Demonstrates basic usage of the MFT C++ library.
 */

#include <iostream>
#include <string>
#include "mft/mft_file.hpp"
#include "mft/common/logger.hpp"
#include "mft/attributes/file_name.hpp"

void print_usage() {
    std::cout << "Usage: mft_example <path_to_mft_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string mft_path = argv[1];
    
    try {
        // Set logging level
        mft::common::Logger::set_level(mft::common::LogLevel::Info);
        
        std::cout << "Loading MFT from: " << mft_path << "\n";
        
        // Load and parse the MFT
        auto mft = mft::MftFile::load(mft_path, false);
        
        std::cout << "MFT Size: " << mft->file_size() << " bytes\n";
        std::cout << "Active File Records: " << mft->file_records().size() << "\n";
        std::cout << "Free File Records: " << mft->free_file_records().size() << "\n";
        std::cout << "Bad Records: " << mft->bad_records().size() << "\n";
        std::cout << "Uninitialized Records: " << mft->uninitialized_records().size() << "\n";
        
        // Print first 10 file records
        std::cout << "\nFirst 10 file records:\n";
        std::cout << std::string(60, '-') << "\n";
        
        int count = 0;
        for (const auto& [key, record] : mft->file_records()) {
            if (count++ >= 10) break;
            
            std::cout << "Key: " << key 
                      << " Entry: " << std::hex << record->entry_number()
                      << " Seq: " << record->sequence_number()
                      << std::dec;
            
            // Find FileName attribute
            for (const auto& attr : record->attributes()) {
                if (attr->attribute_type() == mft::attributes::AttributeType::FileName) {
                    auto* fn = dynamic_cast<const mft::attributes::FileName*>(attr.get());
                    if (fn) {
                        std::cout << " Name: " << fn->file_info().file_name();
                    }
                    break;
                }
            }
            
            if (record->is_directory()) {
                std::cout << " [DIR]";
            }
            
            std::cout << "\n";
        }
        
        std::cout << "\nParsing completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
