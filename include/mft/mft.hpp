#pragma once

#include "mft/file_record.hpp"
#include "mft/other/directory_name_map_value.hpp"
#include <istream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

namespace mft {

/**
 * @brief Main MFT parser class
 * 
 * Reads and parses the entire $MFT file, building maps for
 * file records and directory relationships.
 */
class Mft {
public:
    /**
     * @brief Construct and parse MFT from input stream
     * @param file_stream Input stream containing MFT data
     * @param recover_from_slack Whether to attempt slack space recovery
     */
    Mft(std::istream& file_stream, bool recover_from_slack);
    
    /**
     * @brief Get total file size
     */
    int64_t file_size() const { return file_size_; }
    
    /**
     * @brief Get all active file records
     */
    const std::unordered_map<std::string, std::unique_ptr<FileRecord>>& file_records() const {
        return file_records_;
    }
    
    /**
     * @brief Get all free (deleted) file records
     */
    const std::unordered_map<std::string, std::unique_ptr<FileRecord>>& free_file_records() const {
        return free_file_records_;
    }
    
    /**
     * @brief Get bad records
     */
    const std::vector<std::unique_ptr<FileRecord>>& bad_records() const {
        return bad_records_;
    }
    
    /**
     * @brief Get uninitialized records
     */
    const std::vector<std::unique_ptr<FileRecord>>& uninitialized_records() const {
        return uninitialized_records_;
    }
    
    /**
     * @brief Get current processing offset (for error reporting)
     */
    static int current_offset() { return current_offset_; }
    
    /**
     * @brief Get full parent path for a record key
     * @param record_key Key in format "XXXXXXXX-XXXXXXXX"
     * @return Full path string
     */
    std::string get_full_parent_path(const std::string& record_key) const;
    
    /**
     * @brief Get contents of a directory
     * @param key Directory record key
     * @return List of entries in the directory
     */
    std::vector<other::ParentMapEntry> get_directory_contents(const std::string& key) const;

private:
    int64_t file_size_;
    
    std::unordered_map<std::string, std::unique_ptr<FileRecord>> file_records_;
    std::unordered_map<std::string, std::unique_ptr<FileRecord>> free_file_records_;
    std::unordered_map<std::string, std::vector<FileRecord*>> extension_file_records_;
    std::unordered_map<std::string, std::vector<FileRecord*>> unassociated_extension_records_;
    
    std::vector<std::unique_ptr<FileRecord>> bad_records_;
    std::vector<std::unique_ptr<FileRecord>> uninitialized_records_;
    
    // Directory maps
    std::unordered_map<std::string, std::unique_ptr<other::DirectoryNameMapValue>> directory_name_map_;
    std::unordered_map<std::string, std::unordered_set<other::ParentMapEntry>> parent_directory_name_map_;
    
    static int current_offset_;
    
    void process_extension_blocks();
    void build_maps(const std::unordered_map<std::string, std::unique_ptr<FileRecord>>& records,
                   bool skip_unassociated = true);
};

} // namespace mft
