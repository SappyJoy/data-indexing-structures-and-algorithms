// src/lib/indexes/KDTreeIndex.cpp

#include "kdtree/indexes/KDTreeIndex.hpp"
#include <stdexcept>

namespace kdtree {

// Constructor: Initializes the KDTree
KDTreeIndex::KDTreeIndex() : kdtree_(std::make_unique<KDTree>()) {}

// Build the KD-tree with the provided points
void KDTreeIndex::build(const std::vector<Point> &points) {
    if (!kdtree_) {
        throw std::runtime_error("KDTree instance is not initialized.");
    }
    kdtree_->build(points);
}

// Insert a single point into the KD-tree
void KDTreeIndex::insert(const Point &point) {
    if (!kdtree_) {
        throw std::runtime_error("KDTree instance is not initialized.");
    }
    kdtree_->insert(point);
}

// Find k nearest neighbors using the KD-tree
std::vector<Point> KDTreeIndex::nearest_neighbors(const Point &query, size_t k) const {
    if (!kdtree_) {
        throw std::runtime_error("KDTree instance is not initialized.");
    }
    return kdtree_->nearest_neighbors(query, k);
}

// Range search using the KD-tree
std::vector<Point> KDTreeIndex::range_search(const Point &query, double radius) const {
    if (!kdtree_) {
        throw std::runtime_error("KDTree instance is not initialized.");
    }
    return kdtree_->range_search(query, radius);
}

} // namespace kdtree
