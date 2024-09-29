#ifndef KDTREE_HPP
#define KDTREE_HPP

#include "kdtree/KDNode.hpp"
#include "kdtree/Point.hpp"
#include <memory>
#include <queue>
#include <vector>

namespace kdtree {

struct CompareDistance {
    bool operator()(const std::pair<double, Point> &a, const std::pair<double, Point> &b) const {
        return a.first < b.first;
    }
};

class KDTree {
  public:
    KDTree();
    explicit KDTree(const std::vector<Point> &points);

    void insert(const Point &point);
    void build(const std::vector<Point> &points);

    std::vector<Point> nearest_neighbors(const Point &query, size_t k) const;
    std::vector<Point> range_search(const Point &query, double radius) const;

    inline size_t dimension() const { return dimension_; }

  private:
    std::shared_ptr<KDNode> root_;
    size_t dimension_;

    std::shared_ptr<KDNode> build_tree(std::vector<Point>::iterator begin, std::vector<Point>::iterator end,
                                       size_t depth);
    std::shared_ptr<KDNode> insert_point(std::shared_ptr<KDNode> node, const Point &point, size_t depth);
    void nearest_neighbors(const std::shared_ptr<KDNode> &node, const Point &query, size_t k,
                           std::priority_queue<std::pair<double, Point>, std::vector<std::pair<double, Point>>,
                                               CompareDistance> &best_points) const;
    void range_search(const std::shared_ptr<KDNode> &node, const Point &query, double radius,
                      std::vector<Point> &results) const;
    double distance(const Point &a, const Point &b) const;
};

} // namespace kdtree

#endif
