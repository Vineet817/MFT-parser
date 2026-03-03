#include "i30/i30.hpp"
#include "mft/common/byte_reader.hpp"
#include "mft/common/logger.hpp"
#include "mft/fixup_data.hpp"
#include <fstream>
#include <unordered_set>

namespace i30 {

using mft::common::ByteReader;
using mft::common::Logger;
using mft::FixupData;

I30::I30(std::istream& file_stream) {
    constexpr int page_size = 0x1000;
    constexpr int32_t expected_sig = 0x58444E49;  // "INDX"
    
    std::vector<std::vector<uint8_t>> pages;
    
    // Read all pages
    while (file_stream.good()) {
        std::vector<uint8_t> page(page_size);
        file_stream.read(reinterpret_cast<char*>(page.data()), page_size);
        
        if (file_stream.gcount() == page_size) {
            pages.push_back(std::move(page));
        } else {
            break;
        }
    }
    
    std::unordered_set<std::string> unique_slack_md5s;
    
    int page_number = 0;
    for (const auto& page : pages) {
        Logger::debug_fmt("Processing page 0x{:X}", page_number);
        
        int32_t sig_actual = ByteReader::read_int32(page.data(), 0);
        
        if (sig_actual == 0x00) {
            Logger::warning_fmt("Empty page found at offset 0x{:X}. Skipping",
                page_number * 0x1000);
            page_number++;
            continue;
        }
        
        if (sig_actual != expected_sig) {
            throw std::runtime_error("Invalid header! Expected 'INDX' Signature");
        }
        
        int16_t fixup_offset = ByteReader::read_int16(page.data(), 4);
        int16_t num_fixup_pairs = ByteReader::read_int16(page.data(), 6);
        
        // Skip LSN and VCN
        int32_t data_start_offset = ByteReader::read_int32(page.data(), 0x18);
        int32_t data_size = ByteReader::read_int32(page.data(), 0x1C);
        
        size_t fixup_total_length = static_cast<size_t>(num_fixup_pairs) * 2;
        std::vector<uint8_t> fixup_buffer(fixup_total_length);
        ByteReader::block_copy(page.data(), fixup_offset, fixup_buffer.data(), 0, fixup_total_length);
        
        FixupData fixup_data(fixup_buffer.data(), fixup_total_length);
        
        // Create mutable copy and apply fixups
        std::vector<uint8_t> raw_bytes(page.begin() + 0x18 + data_start_offset, page.end());
        
        // Fixup verification
        int counter = 512 - data_start_offset - 0x18;
        for (const auto& actual_bytes : fixup_data.fixup_actual()) {
            int fix_offset = counter - 2;
            
            if (fix_offset >= 0 && static_cast<size_t>(fix_offset + 2) <= raw_bytes.size()) {
                int16_t expected = ByteReader::read_int16(raw_bytes.data(), fix_offset);
                if (expected != fixup_data.fixup_expected()) {
                    Logger::warning_fmt(
                        "Fixup values do not match at 0x{:X}. Expected: 0x{:X2}, actual: 0x{:X2}",
                        fix_offset, fixup_data.fixup_expected(), expected);
                }
                
                ByteReader::block_copy(actual_bytes.data(), 0, raw_bytes.data(), fix_offset, 2);
            }
            counter += 512;
        }
        
        // Parse active entries
        size_t active_space_size = static_cast<size_t>(data_size - data_start_offset);
        if (active_space_size > raw_bytes.size()) {
            active_space_size = raw_bytes.size();
        }
        
        size_t index = 0;
        while (index < active_space_size) {
            int64_t absolute_offset = page_number * 0x1000 + 0x18 + data_start_offset + index;
            
            Logger::verbose_fmt("IN ACTIVE LOOP: Absolute offset: 0x{:X}", absolute_offset);
            
            if (index + 10 > active_space_size) break;
            
            // Skip MFT info and read index size
            int16_t index_size = ByteReader::read_int16(raw_bytes.data(), index + 8);
            
            if (index_size <= 0 || static_cast<size_t>(index_size) > active_space_size - index) {
                break;
            }
            
            std::vector<uint8_t> entry_buffer(raw_bytes.begin() + index,
                                              raw_bytes.begin() + index + index_size);
            
            mft::other::IndexEntryI30 ie(entry_buffer.data(), entry_buffer.size(),
                                         absolute_offset, page_number, false);
            
            if (ie.mft_reference_self().mft_entry_number() != 0) {
                Logger::debug_fmt("{}", ie.to_string());
                entries_.push_back(std::move(ie));
            }
            
            index += static_cast<size_t>(index_size);
        }
        
        page_number++;
    }
}

const std::string I30File::DATE_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

std::unique_ptr<I30> I30File::load(const std::string& i30_path) {
    std::ifstream file(i30_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("'" + i30_path + "' not found");
    }
    return std::make_unique<I30>(file);
}

} // namespace i30
