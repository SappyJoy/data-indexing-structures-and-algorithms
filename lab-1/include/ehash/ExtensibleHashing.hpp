#ifndef EXTENSIBLEHASHING_HPP
#define EXTENSIBLEHASHING_HPP

#include "Bucket.hpp"
#include <google/protobuf/message.h>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ehash {

// ExtensibleHashing class template
template <typename T> class ExtensibleHashing {
    static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                  "T must be a subclass of google::protobuf::Message");

  private:
    size_t globalDepth; // Tracks the depth of the directory (number of bits used for hashing)

    // Directory that maps hash prefixes to buckets and tracks their local depth
    struct DirectoryEntry {
        std::shared_ptr<Bucket<T>> bucket;
        size_t localDepth;
        size_t rootBucketIndex;
    };

    std::unordered_map<size_t, std::shared_ptr<DirectoryEntry>> directories;
    std::string bucketDirectory; // Path where the bucket files are stored
    size_t maxBucketSize;        // Maximum size of each bucket (multiple of block size)

    // Hash function (using basic std::hash)
    size_t hashKey(const std::string &key) const { return std::hash<std::string>{}(key); }

    // Get the hash prefix (using given depth)
    size_t getHashPrefix(size_t hashValue, size_t depth) const {
        return hashValue & ((1 << depth) - 1); // Mask hashValue to use only depth bits
    }

    // Split the bucket and redistribute entries
    void splitBucket(size_t bucketIndex) {
        auto oldBucketEntry = directories[bucketIndex];
        size_t localDepth = oldBucketEntry->localDepth;

        localDepth++;
        directories[bucketIndex]->localDepth = localDepth;

        size_t newBucketIndex = bucketIndex + (1 << (localDepth - 1));
        std::string newBucketPath = bucketDirectory + "/bucket_" + std::to_string(newBucketIndex) + ".dat";
        auto newBucket = std::make_shared<Bucket<T>>(newBucketPath, maxBucketSize);

        auto oldBucket = oldBucketEntry->bucket;
        auto entries = oldBucket->retrieveEntries();
        oldBucket->clear(); // Clear old bucket after moving its entries

        for (auto &entry : entries) {
            std::string serializedKey;
            entry->SerializeToString(&serializedKey);
            size_t hashValue = hashKey(serializedKey);

            size_t newPrefix = getHashPrefix(hashValue, localDepth);
            if (newPrefix == bucketIndex) {
                oldBucket->addEntry(std::move(entry)); // Keep entry in the old bucket
            } else {
                newBucket->addEntry(std::move(entry)); // Move entry to the new bucket
            }
        }

        if (localDepth > globalDepth) {
            globalDepth++; // Increase global depth

            // Double the directory size, referencing the old DirectoryEntry
            size_t oldSize = 1 << (globalDepth - 1); // Half the size of the new directory
            for (size_t i = 0; i < oldSize; ++i) {
                // Create references to the existing directory entries
                directories[i + oldSize] = directories[i];
            }
        }
        directories[newBucketIndex] =
            std::make_shared<DirectoryEntry>(DirectoryEntry{newBucket, localDepth, newBucketIndex});
        // Update the directory with the new bucket
        for (size_t i = newBucketIndex + (1 << localDepth); i < ((size_t)1 << globalDepth); i += (1 << localDepth)) {
            directories[i] = directories[newBucketIndex];
        }
    }

    size_t addEntryInternal(std::unique_ptr<T> entry, std::size_t entrySize, std::size_t hashValue) {
        size_t bucketIndex = getHashPrefix(hashValue, globalDepth);
        auto &targetBucketEntry = directories[bucketIndex];
        auto &targetBucket = targetBucketEntry->bucket;

        // Try to add the entry
        if (!targetBucket->canAddEntry(entrySize)) {
            splitBucket(targetBucketEntry->rootBucketIndex);

            // Retry adding the entry after the split
            return addEntryInternal(std::move(entry), entrySize, hashValue);
        }

        targetBucket->addEntry(std::move(entry));

        return hashValue;
    }

  public:
    // Constructor
    ExtensibleHashing(const std::string &directoryPath, size_t bucketSize)
        : ExtensibleHashing(directoryPath, bucketSize, 1) {}

    ExtensibleHashing(const std::string &directoryPath, size_t bucketSize, size_t initialGlobalDepth)
        : globalDepth(initialGlobalDepth), bucketDirectory(directoryPath), maxBucketSize(bucketSize) {
        // Initialize the directory with empty buckets
        for (size_t i = 0; i < ((size_t)1 << globalDepth); ++i) {
            std::string bucketPath = bucketDirectory + "/bucket_" + std::to_string(i) + ".dat";
            directories[i] = std::make_shared<DirectoryEntry>(
                DirectoryEntry{std::make_shared<Bucket<T>>(bucketPath, maxBucketSize), globalDepth, i});
        }
    }

    // Add entry to the hash table
    size_t addEntry(std::unique_ptr<T> entry) {
        // Serialize the key once
        std::string serializedKey;
        entry->SerializeToString(&serializedKey);

        size_t hashValue = hashKey(serializedKey);
        size_t bucketIndex = getHashPrefix(hashValue, globalDepth);

        auto &targetBucketEntry = directories[bucketIndex];
        auto &targetBucket = targetBucketEntry->bucket;

        if (targetBucket->hasKey(serializedKey)) {
            targetBucket->updateEntry(std::move(entry));
            return hashValue;
        }

        // Try to add the entry
        if (!targetBucket->canAddEntry(serializedKey.size())) {

            splitBucket(targetBucketEntry->rootBucketIndex);

            // Retry adding the entry after the split
            return addEntryInternal(std::move(entry), serializedKey.size(), hashValue);
        }

        targetBucket->addEntry(std::move(entry));

        return hashValue;
    }

    const std::vector<std::unique_ptr<T>> &getEntries(const std::unique_ptr<T> entry) const {
        std::string serializedKey;
        entry->SerializeToString(&serializedKey);

        size_t hashValue = hashKey(serializedKey);
        size_t bucketIndex = getHashPrefix(hashValue, globalDepth);
        return directories.at(bucketIndex)->bucket->getEntries();
    }

    // Retrieve entries in a bucket for testing or debugging
    const std::vector<std::unique_ptr<T>> &getEntries(const size_t &hash) const {
        size_t bucketIndex = getHashPrefix(hash, globalDepth);
        return directories.at(bucketIndex)->bucket->getEntries();
    }

    std::optional<T *> getEntry(const size_t &hash) const {
        size_t bucketIndex = getHashPrefix(hash, globalDepth);
        const auto &entries = directories.at(bucketIndex)->bucket->getEntries(); // Get entries from the correct bucket

        for (const auto &entry : entries) {
            std::string serializedKey;
            entry->SerializeToString(&serializedKey); // Serialize the entry to compare the hash

            // Check if the hash of this entry matches the input hash
            if (hashKey(serializedKey) == hash) {
                return entry.get(); // Return raw pointer (no ownership change)
            }
        }

        return std::nullopt; // If no entry is found, return std::nullopt
    }

    void print() const {
        for (const auto &pair : directories) {
            std::cout << "Bucket Index: " << pair.first << " Depth: " << pair.second->localDepth << " {" << std::endl;
            pair.second->bucket->print();
            std::cout << "}" << std::endl;
        }
    }

    size_t bucketCount() const { return directories.size(); }
};

} // namespace ehash

#endif
