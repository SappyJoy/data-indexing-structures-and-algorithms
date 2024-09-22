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
        // std::cout << "Splitting bucket at index: " << bucketIndex << " into two buckets: " << newBucketIndex << " and
        // "
        //           << bucketIndex << "" << std::endl;

        auto oldBucket = oldBucketEntry->bucket;
        auto entries = oldBucket->retrieveEntries();
        oldBucket->clear(); // Clear old bucket after moving its entries

        for (auto &entry : entries) {
            std::string serializedKey;
            entry->SerializeToString(&serializedKey);
            size_t hashValue = hashKey(serializedKey);
            // std::cout << "Entry " << entry->DebugString() << "Hash value: " << hashValue << std::endl;

            size_t newPrefix = getHashPrefix(hashValue, localDepth);
            if (newPrefix == bucketIndex) {
                // std::cout << "Moved entry " << entry->DebugString() << " in the old bucket " << bucketIndex
                //           << std::endl;
                oldBucket->addEntry(std::move(entry)); // Keep entry in the old bucket
            } else {
                // std::cout << "Moved entry " << entry->DebugString() << " to the new bucket " << newBucketIndex
                //           << " but new prefix " << newPrefix << std::endl;
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
        for (size_t i = newBucketIndex + (1 << localDepth); i < (1 << globalDepth); i += (1 << localDepth)) {
            directories[i] = directories[newBucketIndex];
        }
    }

  public:
    // Constructor
    ExtensibleHashing(const std::string &directoryPath, size_t bucketSize)
        : ExtensibleHashing(directoryPath, bucketSize, 1) {}

    ExtensibleHashing(const std::string &directoryPath, size_t bucketSize, size_t initialGlobalDepth)
        : globalDepth(initialGlobalDepth), bucketDirectory(directoryPath), maxBucketSize(bucketSize) {
        // Initialize the directory with empty buckets
        for (size_t i = 0; i < (1 << globalDepth); ++i) {
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
        // std::cout << "Serialized key" << std::endl;

        size_t hashValue = hashKey(serializedKey);
        size_t bucketIndex = getHashPrefix(hashValue, globalDepth);
        // if (bucketIndex >= 30) {
        //     throw std::runtime_error("Bucket index out of range");
        //     return 0;
        // }

        // std::cout << "Hash value: " << hashValue << " Bucket index: " << bucketIndex << std::endl;
        // std::cout << "Add new entry " << entry->DebugString() << " Hash value: " << hashValue
        //           << " Bucket index: " << bucketIndex << std::endl;

        auto &targetBucketEntry = directories[bucketIndex];
        auto &targetBucket = targetBucketEntry->bucket;
        // std::cout << "Target bucket retrieved" << std::endl;

        // Try to add the entry
        if (!targetBucket->addEntry(std::move(entry))) {

            // Perform bucket split
            // print();

            splitBucket(targetBucketEntry->rootBucketIndex);

            // Create a new unique_ptr from the original entry's serialized string
            auto newEntry = std::make_unique<T>();
            newEntry->ParseFromString(serializedKey); // Deserialize the object from the serialized key

            // Retry adding the entry after the split
            return addEntry(std::move(newEntry));
        }
        // print();

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
        // std::cout << "Bucket index: " << bucketIndex << std::endl;
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
