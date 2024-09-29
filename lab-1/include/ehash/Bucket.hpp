#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <fstream>
#include <google/protobuf/message.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/statvfs.h>
#include <vector>

namespace ehash {

// Default block size (4KB)
// const size_t DEFAULT_BLOCK_SIZE = 4096;

// Helper function to check if a file exists
bool fileExists(const std::string &path) {
    std::ifstream file(path);
    return file.good();
}

// Helper function to get filesystem block size for a given path
size_t getBlockSize(const std::string &path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        return stat.f_bsize; // Return block size
    } else {
        throw std::runtime_error("Unable to get block size for path: " + path);
    }
}

// Create the file if it doesn't exist, and open it
void createFileIfNotExists(const std::string &path) {
    // std::cout << "createFileIfNotExists" << std::endl;
    if (!fileExists(path)) {
        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("Failed to create bucket file: " + path);
        }
        file.close();
    }
    // std::cout << "File EXISTS" << std::endl;
}

// Generic Bucket class for storing any Protobuf objects
template <typename T> class Bucket {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                  "T must be a subclass of google::protobuf::Message");

  private:
    std::string filePath;                    // Path to the file where the bucket is stored
    size_t blockSize;                        // Filesystem block size (e.g., 4KB)
    size_t maxBucketSize;                    // Maximum size of the bucket (a multiple of block size)
    std::vector<std::unique_ptr<T>> entries; // Deserialized objects in memory
    std::size_t currentSize = 0;             // Current size of the bucket

    // Internal method to serialize and write to disk
    void writeToDisk() {
        std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
        if (!outFile) {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }

        size_t currentSize = 0;
        for (const auto &entry : entries) {
            std::string serializedEntry;
            entry->SerializeToString(&serializedEntry);
            size_t entrySize = serializedEntry.size();

            // Check if adding this entry would exceed the bucket's size limit
            if (currentSize + sizeof(int) + entrySize > maxBucketSize) {
                throw std::runtime_error("Bucket overflow: adding entry exceeds max bucket size");
            }

            outFile.write(reinterpret_cast<const char *>(&entrySize), sizeof(int)); // Write size of entry
            outFile.write(serializedEntry.data(), entrySize);                       // Write serialized data
            currentSize += sizeof(int) + entrySize;                                 // Update current bucket size
        }

        // Pad the rest of the bucket to ensure it's a multiple of the block size
        size_t padding = maxBucketSize - currentSize;
        std::string paddingData(padding, '\0');
        outFile.write(paddingData.data(), padding);
        outFile.close();
    }

    // Internal method to read and deserialize from disk
    void readFromDisk() {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile) {
            throw std::runtime_error("Failed to open file for reading: " + filePath);
        }

        entries.clear();
        size_t currentSize = 0;
        while (currentSize < maxBucketSize && !inFile.eof()) {
            int entrySize = 0;
            inFile.read(reinterpret_cast<char *>(&entrySize), sizeof(int));
            if (inFile.eof())
                break;

            std::string serializedEntry(entrySize, '\0');
            inFile.read(&serializedEntry[0], entrySize);
            if (inFile.eof())
                break;

            // Create a new instance of T (a Protobuf Message)
            std::unique_ptr<T> entry(new T());
            if (!entry->ParseFromString(serializedEntry)) {
                throw std::runtime_error("Failed to parse protobuf object");
            }
            entries.push_back(std::move(entry));
            currentSize += sizeof(int) + entrySize;
        }

        inFile.close();
    }

  public:
    Bucket(const std::string &path, size_t maxSize) : filePath(path), currentSize(0) {
        // Create the file if it doesn't exist
        createFileIfNotExists(path);

        // Now safely get the block size
        blockSize = getBlockSize(path);

        // Ensure maxBucketSize is a multiple of the block size
        maxBucketSize = ((maxSize / blockSize) + 1) * blockSize;
        if (maxBucketSize < blockSize) {
            throw std::runtime_error("Max bucket size must be at least one block size");
        }

        readFromDisk(); // Load objects into memory when bucket is initialized
    }

    ~Bucket() = default;

    // Add a new Protobuf entry to the bucket
    bool addEntry(std::unique_ptr<T> entry) {
        // std::cout << "Adding entry to bucket" << std::endl;
        std::string serializedEntry;
        entry->SerializeToString(&serializedEntry);
        size_t entrySize = serializedEntry.size();

        // If the entry itself is larger than the maximum bucket size, throw an error
        if (entrySize > maxBucketSize) {
            throw std::runtime_error("Entry size exceeds maximum bucket size");
        }

        if (currentSize + sizeof(int) + entrySize > maxBucketSize) {
            return false; // Bucket full, cannot add more entries
        }

        entries.push_back(std::move(entry));
        currentSize += sizeof(int) + entrySize;
        writeToDisk(); // Persist to disk after modification
        return true;
    }

    bool canAddEntry(std::size_t entrySize) const { return currentSize + sizeof(int) + entrySize <= maxBucketSize; }

    bool hasKey(const std::string &key) const {
        for (const auto &entry : entries) {
            std::string serializedKey;
            entry->SerializeToString(&serializedKey);
            if (key == serializedKey) {
                return true;
            }
        }
        return false;
    }

    void updateEntry(std::unique_ptr<T> newEntry) {
        std::string serializedKey;
        newEntry->SerializeToString(&serializedKey);

        for (auto &entry : entries) {
            std::string existingKey;
            entry->SerializeToString(&existingKey);

            if (existingKey == serializedKey) {
                entry = std::move(newEntry); // Replace the existing entry
                return;
            }
        }
    }

    // Retrieve all entries from the bucket
    const std::vector<std::unique_ptr<T>> &getEntries() const { return entries; }

    std::vector<std::unique_ptr<T>> retrieveEntries() { return std::move(entries); }

    // Check if bucket is full
    bool isFull() const {
        size_t currentSize = 0;
        for (const auto &entry : entries) {
            std::string serializedEntry;
            entry->SerializeToString(&serializedEntry);
            currentSize += sizeof(int) + serializedEntry.size();
        }
        return currentSize >= maxBucketSize;
    }

    void clear() {
        entries.clear();
        currentSize = 0;
        writeToDisk();
    }

    void print() const {
        for (const auto &entry : entries) {
            std::cout << "Entry: " << entry->DebugString();
        }
    }
};

} // namespace ehash

#endif
