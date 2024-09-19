#ifndef EXTENSIBLE_HASHING_HPP
#define EXTENSIBLE_HASHING_HPP

#include "Bucket.hpp"
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace ehash {

// ExtensibleHashing class representing the hash table
class ExtensibleHashing {
    std::size_t global_depth;
    std::size_t bucket_capacity;
    std::vector<std::shared_ptr<Bucket>> directory;

    int hash(int key) const;

    void split_bucket(std::size_t bucketIndex);

  public:
    ExtensibleHashing(std::size_t bucket_capacity);

    bool insert(int key, int value);

    bool remove(int key);

    std::optional<int> search(int key);

    void print() const;
};

} // namespace ehash

#endif
