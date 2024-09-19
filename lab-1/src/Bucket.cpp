#include "Bucket.hpp"
#include <optional>
#include <stdexcept>

namespace ehash {

Bucket::Bucket(std::size_t capacity) : local_depth(capacity) {}

bool Bucket::is_full() const { return data.size() >= local_depth; }

bool Bucket::insert(int key, int value) {
    if (is_full())
        return false;
    data[key] = value;
    return true;
}

bool Bucket::remove(int key) {
    if (data.find(key) == data.end())
        return false;
    data.erase(key);
    return true;
}

std::optional<int> Bucket::search(int key) const {
    auto it = data.find(key);
    if (it != data.end())
        return it->second;
    return std::nullopt;
}

void Bucket::clear() { data.clear(); }

const std::unordered_map<int, int> &Bucket::get_data() const { return data; }

} // namespace ehash
