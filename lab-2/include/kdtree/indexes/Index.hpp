#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>

namespace kdtree {

class Point;

class Index {
  public:
    virtual ~Index() = default;

    virtual void build(const std::vector<Point> &points) = 0;

    virtual void insert(const Point &point) = 0;

    virtual std::vector<Point> nearest_neighbors(const Point &query, std::size_t k) const = 0;

    virtual std::vector<Point> range_search(const Point &query, double radius) const = 0;
};

} // namespace kdtree

#endif
