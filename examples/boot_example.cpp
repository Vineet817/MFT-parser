/**
 * Boot Sector Parser Example
 * 
 * Demonstrates parsing of $Boot file.
 */

#include <iostream>
#include <string>
#include <iomanip>
#include "boot/boot.hpp"

void print_usage() {
    std::cout << "Usage: boot_example <path_to_boot_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    std::string boot_path = argv[1];
    
    try {
        std::cout << "Loading Boot sector from: " << boot_path << "\n\n";
        
        auto boot = boot::BootFile::load(boot_path);
        
        std::cout << "=== NTFS Boot Sector Information ===\n\n";
        
        std::cout << "File System: " << boot->file_system_signature() << "\n";
        std::cout << "Boot Entry Point: " << boot->boot_entry_point() << "\n";
        std::cout << "Sector Signature: " << boot->get_sector_signature() << "\n\n";
        
        std::cout << "=== BPB (BIOS Parameter Block) ===\n";
        std::cout << "Bytes Per Sector: " << boot->bytes_per_sector() << "\n";
        std::cout << "Sectors Per Cluster: " << boot->sectors_per_cluster() << "\n";
        std::cout << "Media Descriptor: 0x" << std::hex << (int)boot->media_descriptor() 
                  << std::dec << " (" << boot->decode_media_descriptor() << ")\n\n";
        
        std::cout << "=== NTFS-Specific Fields ===\n";
        std::cout << "Total Sectors: " << boot->total_sectors() << "\n";
        std::cout << "MFT Cluster Block Number: " << boot->mft_cluster_block_number() << "\n";
        std::cout << "MFT Mirror Cluster Block Number: " << boot->mirror_mft_cluster_block_number() << "\n";
        std::cout << "MFT Entry Size: " << boot->mft_entry_size() << " bytes\n";
        std::cout << "Index Entry Size: " << boot->index_entry_size() << " bytes\n";
        std::cout << "Volume Serial Number: " << boot->get_volume_serial_number() << "\n";
        
        // Calculate some useful values
        int64_t cluster_size = boot->bytes_per_sector() * boot->sectors_per_cluster();
        int64_t mft_offset = boot->mft_cluster_block_number() * cluster_size;
        int64_t volume_size = boot->total_sectors() * boot->bytes_per_sector();
        
        std::cout << "\n=== Calculated Values ===\n";
        std::cout << "Cluster Size: " << cluster_size << " bytes\n";
        std::cout << "MFT Offset: " << mft_offset << " bytes (0x" 
                  << std::hex << mft_offset << std::dec << ")\n";
        std::cout << "Volume Size: " << (volume_size / (1024 * 1024 * 1024)) << " GB\n";
        
        std::cout << "\nParsing completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
