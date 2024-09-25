#ifndef KDTREE_INDEX_HPP
#define KDTREE_INDEX_HPP

#include "Index.hpp"
#include "kdtree/KDTree.hpp"

namespace index {

class KDTreeIndex : public Index {
public:
    KDTreeIndex();

    void build(const std::vector<Point>& points) override;
    void insert(const Point& point) override;
    std::vector<Point> nearest_neighbors(const Point& query, size_t k) const override;
    std::vector<Point> range_search(const Point& query, double radius) const override;

private:
    kdtree::KDTree tree_;
};

}  // namespace index

#endif
