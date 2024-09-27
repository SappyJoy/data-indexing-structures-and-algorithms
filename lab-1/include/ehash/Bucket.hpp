#ifndef BUCKET_HPP
#define BUCKET_HPP

#include "log/Logger.hpp"
#include <fstream>
#include <google/protobuf/message.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sys/statvfs.h>
#include <unordered_map>
#include <vector>

namespace ehash {

template <typename T> class Bucket {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                  "T must be a subclass of google::protobuf::Message");

  private:
    std::string filePath;
    size_t blockSize;
    size_t maxBucketSize;
    std::vector<std::unique_ptr<T>> entries;
    std::size_t currentSize = 0;
    bool isDirty = false;
    std::unordered_map<size_t, T *> hashMap;
    mutable std::mutex mtx;

    void writeToDisk() {
        std::lock_guard<std::mutex> lock(mtx);
        std::string tempFilePath = filePath + ".tmp";
        std::ofstream outFile(tempFilePath, std::ios::binary | std::ios::trunc);
        if (!outFile) {
            Logger::error("Failed to open temp file for writing: {}", tempFilePath);
            throw std::runtime_error("Failed to open temp file for writing: " + tempFilePath);
        }

        size_t currentSizeLocal = 0;
        for (const auto &entry : entries) {
            std::string serializedEntry;
            entry->SerializeToString(&serializedEntry);
            size_t entrySize = serializedEntry.size();
            size_t entryHash = hashKey(keyExtractor(*entry));

            if (currentSizeLocal + sizeof(int) + entrySize > maxBucketSize) {
                throw std::runtime_error("Bucket overflow: adding entry exceeds max bucket size");
            }

            outFile.write(reinterpret_cast<const char *>(&entrySize), sizeof(int));
            outFile.write(serializedEntry.data(), entrySize);
            currentSizeLocal += sizeof(int) + entrySize;
        }

        // Padding
        size_t padding = maxBucketSize - currentSizeLocal;
        std::string paddingData(padding, '\0');
        outFile.write(paddingData.data(), padding);
        outFile.close();

        // Atomic replace
        if (std::rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
            Logger::error("Failed to rename temp file to: {}", filePath);
            throw std::runtime_error("Failed to rename temp file to: " + filePath);
        }

        currentSize = currentSizeLocal;
        isDirty = false;
    }

    void readFromDisk() {
        std::lock_guard<std::mutex> lock(mtx);
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile) {
            Logger::error("Failed to open file for reading: {}", filePath);
            throw std::runtime_error("Failed to open file for reading: " + filePath);
        }

        entries.clear();
        hashMap.clear();
        size_t currentSizeLocal = 0;
        while (currentSizeLocal < maxBucketSize && !inFile.eof()) {
            int entrySize = 0;
            inFile.read(reinterpret_cast<char *>(&entrySize), sizeof(int));
            if (inFile.eof())
                break;

            std::string serializedEntry(entrySize, '\0');
            inFile.read(&serializedEntry[0], entrySize);
            if (inFile.eof())
                break;

            std::unique_ptr<T> entry(new T());
            if (!entry->ParseFromString(serializedEntry)) {
                Logger::error("Failed to parse protobuf object in file: {}", filePath);
                throw std::runtime_error("Failed to parse protobuf object");
            }

            size_t entryHash = hashKey(keyExtractor(*entry));
            hashMap[entryHash] = entry.get();
            entries.push_back(std::move(entry));
            currentSizeLocal += sizeof(int) + entrySize;
        }

        currentSize = currentSizeLocal;
        inFile.close();
    }

  public:
    // Key extractor function
    std::function<std::string(const T &)> keyExtractor;

    // Constructor
    Bucket(const std::string &path, size_t maxSize, std::function<std::string(const T &)> extractor)
        : filePath(path), keyExtractor(extractor) {
        createFileIfNotExists(path);
        blockSize = getBlockSize(path);
        maxBucketSize = ((maxSize / blockSize) + 1) * blockSize;
        if (maxBucketSize < blockSize) {
            throw std::runtime_error("Max bucket size must be at least one block size");
        }
        readFromDisk();
    }

    ~Bucket() {
        try {
            persist();
        } catch (const std::exception &e) {
            Logger::error("Bucket destructor failed: {}", e.what());
        }
    }

    bool addEntry(std::unique_ptr<T> entry) {
        std::lock_guard<std::mutex> lock(mtx);
        std::string key = keyExtractor(*entry);
        size_t entryHash = hashKey(key);

        if (hashMap.find(entryHash) != hashMap.end()) {
            return false; // Duplicate key
        }

        std::string serializedEntry;
        entry->SerializeToString(&serializedEntry);
        size_t entrySize = serializedEntry.size();

        if (currentSize + sizeof(int) + entrySize > maxBucketSize) {
            return false; // Bucket full
        }

        hashMap[entryHash] = entry.get();
        entries.push_back(std::move(entry));
        currentSize += sizeof(int) + entrySize;
        isDirty = true;
        return true;
    }

    bool canAddEntry(std::size_t entrySize) const { return (currentSize + sizeof(int) + entrySize) <= maxBucketSize; }

    bool hasKey(const std::string &key) const {
        size_t entryHash = hashKey(key);
        std::lock_guard<std::mutex> lock(mtx);
        return hashMap.find(entryHash) != hashMap.end();
    }

    void updateEntry(std::unique_ptr<T> newEntry) {
        std::lock_guard<std::mutex> lock(mtx);
        std::string key = keyExtractor(*newEntry);
        size_t entryHash = hashKey(key);

        auto it = hashMap.find(entryHash);
        if (it != hashMap.end()) {
            // Find and replace the entry
            for (auto &entry : entries) {
                if (hashKey(keyExtractor(*entry)) == entryHash) {
                    entry = std::move(newEntry);
                    isDirty = true;
                    break;
                }
            }
        }
    }

    const std::vector<std::unique_ptr<T>> &getEntries() const { return entries; }

    std::vector<std::unique_ptr<T>> retrieveEntries() { return std::move(entries); }

    bool isFull() const { return currentSize >= maxBucketSize; }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        entries.clear();
        hashMap.clear();
        currentSize = 0;
        isDirty = true;
    }

    void persist() {
        if (isDirty) {
            writeToDisk();
        }
    }

    T *getEntry(size_t hash) const {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = hashMap.find(hash);
        if (it != hashMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    void print() const {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto &entry : entries) {
            std::cout << "Entry: " << entry->DebugString();
        }
    }
};

} // namespace ehash

#endif
