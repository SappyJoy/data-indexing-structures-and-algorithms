#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <optional>
#include <unordered_map>

namespace ehash {

// Bucket class representing a single bucket in the hash table
class Bucket {
    std::unordered_map<int, int> data;
    std::size_t local_depth;

  public:
    explicit Bucket(std::size_t capacity);

    bool is_full() const;

    bool insert(int key, int value);

    bool remove(int key);

    std::optional<int> search(int key) const;

    void clear();

    const std::unordered_map<int, int> &get_data() const;
};

} // namespace ehash

#endif
