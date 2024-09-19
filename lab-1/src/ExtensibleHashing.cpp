#include "ExtensibleHashing.hpp"
#include <iostream>
#include <optional>

namespace ehash {

ExtensibleHashing::ExtensibleHashing(std::size_t bucket_capacity) : global_depth(1), bucket_capacity(bucket_capacity) {
    directory.resize(1 << global_depth);
    for (auto &bucket : directory) {
        bucket = std::make_shared<Bucket>(bucket_capacity);
    }
}

int ExtensibleHashing::hash(int key) const { return key & ((1 << global_depth) - 1); }

void ExtensibleHashing::split_bucket(std::size_t bucketIndex) {
    auto old_bucket = directory[bucketIndex];
    std::size_t localDepth = std::log2(directory.size()) - global_depth;

    global_depth++;
    directory.resize(1 << global_depth);

    for (std::size_t i = 0; i < directory.size(); ++i) {
        directory[i] = (i & ((1 << (global_depth - 1)) - 1)) == bucketIndex ? old_bucket
                                                                            : std::make_shared<Bucket>(bucket_capacity);
    }

    auto temp_data = old_bucket->get_data();
    old_bucket->clear();
    // Move old bucket's data into new buckets after the split
    for (const auto &[key, value] : temp_data) {
        insert(key, value);
    }
}

bool ExtensibleHashing::insert(int key, int value) {
    int hash_index = hash(key);
    auto target_bucket = directory[hash_index];

    if (target_bucket->insert(key, value)) {
        return true;
    }

    // If the bucket is full, split it and rehash the values
    split_bucket(hash_index);
    return insert(key, value);
}

bool ExtensibleHashing::remove(int key) {
    int hash_index = hash(key);
    return directory[hash_index]->remove(key);
}

std::optional<int> ExtensibleHashing::search(int key) {
    int hash_index = hash(key);
    return directory[hash_index]->search(key);
}

void ExtensibleHashing::print() const {
    std::cout << "Global Depth: " << global_depth << "\n";
    for (std::size_t i = 0; i < directory.size(); ++i) {
        std::cout << "Bucket " << i << ": ";
        for (const auto &[key, value] : directory[i]->get_data()) {
            std::cout << "(" << key << ", " << value << ") ";
        }
        std::cout << "\n";
    }
}
} // namespace ehash
