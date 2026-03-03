#pragma once

#include <string>
#include <functional>

namespace mft {
namespace other {

/**
 * @brief Value type for directory name mapping
 */
class DirectoryNameMapValue {
public:
    DirectoryNameMapValue(std::string name, std::string parent_record_key, bool is_deleted)
        : name_(std::move(name)),
          parent_record_key_(std::move(parent_record_key)),
          is_deleted_(is_deleted) {}
    
    const std::string& name() const { return name_; }
    const std::string& parent_record_key() const { return parent_record_key_; }
    bool is_deleted() const { return is_deleted_; }

private:
    std::string name_;
    std::string parent_record_key_;
    bool is_deleted_;
};

/**
 * @brief Entry in the parent directory map
 */
class ParentMapEntry {
public:
    ParentMapEntry(std::string file_name, std::string record_key, bool is_directory)
        : file_name_(std::move(file_name)),
          record_key_(std::move(record_key)),
          is_directory_(is_directory) {}
    
    const std::string& file_name() const { return file_name_; }
    const std::string& record_key() const { return record_key_; }
    bool is_directory() const { return is_directory_; }
    
    bool operator==(const ParentMapEntry& other) const {
        return file_name_ == other.file_name_ && 
               record_key_ == other.record_key_ &&
               is_directory_ == other.is_directory_;
    }

private:
    std::string file_name_;
    std::string record_key_;
    bool is_directory_;
};

} // namespace other
} // namespace mft

// Hash function for ParentMapEntry to be used in unordered_set
namespace std {
    template<>
    struct hash<mft::other::ParentMapEntry> {
        size_t operator()(const mft::other::ParentMapEntry& entry) const {
            size_t h1 = hash<string>{}(entry.file_name());
            size_t h2 = hash<string>{}(entry.record_key());
            return h1 ^ (h2 << 1);
        }
    };
}
