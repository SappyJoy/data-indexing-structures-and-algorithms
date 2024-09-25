#ifndef KDTREE_HPP
#define KDTREE_HPP

#include "kdtree/Point.hpp"
#include <vector>

namespace kdtree {

class KDTree {
public:
    // Constructors
    KDTree();
    explicit KDTree(cons std::vector<Point>& points);

    // Modifiers
    void insert(const Point& point);
    void build(const std::vector<Point>& points);

    // Query Methods
    std::vector<Point> nearest_neighbors(const Point& query, size_t k) const;
    std::vector<Point> range_search(const Point& query, double radius) const;

private:
    std::shared_ptr<KDNode> root_;
    size_t dimension_;

    // Helper Functions
    std::shared_ptr<KDNode> build_tree(std::vector<Point>::iterator begin,
                                       std::vector<Point>::iterator end,
                                       size_t depth);
    void nearest_neighbors(const std::shared_ptr<KDNode>& node,
                           const Point& query,
                           size_t k,
                           std::vector<std::pair<double, Point>>& best_points) const;
    void range_search(const std::shared_ptr<KDNode>& node,
                      const Point& query,
                      double radius,
                      std::vector<Point>& results) const;
};

}  // namespace kdtree

#endif
