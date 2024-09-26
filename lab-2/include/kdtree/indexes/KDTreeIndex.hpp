#ifndef KDTREE_INDEX_HPP
#define KDTREE_INDEX_HPP

#include "Index.hpp"
#include "kdtree/KDTree.hpp"

namespace kdtree {

class KDTreeIndex : public Index {
  public:
    KDTreeIndex();
    ~KDTreeIndex() override = default;

    // Build the KD-tree index with a set of points
    void build(const std::vector<Point> &points) override;

    // Insert a single point into the KD-tree index
    void insert(const Point &point) override;

    // Find k nearest neighbors to the query point using KD-tree
    std::vector<Point> nearest_neighbors(const Point &query, size_t k) const override;

    // Range search: find all points within radius using KD-tree
    std::vector<Point> range_search(const Point &query, double radius) const override;

  private:
    std::unique_ptr<KDTree> kdtree_;
};

} // namespace kdtree

#endif
