#ifndef INDEX_HPP
#define INDEX_HPP

#include <vector>

class Point;  // Forward declaration

namespace kdtree {
namespace index {

class Index {
public:
    virtual ~Index() = default;

    // Build the index with a set of points
    virtual void build(const std::vector<Point>& points) = 0;

    // Insert a single point into the index
    virtual void insert(const Point& point) = 0;

    // k-Nearest Neighbors query
    virtual std::vector<Point> nearest_neighbors(const Point& query, std::size_t k) const = 0;

    // Range search query
    virtual std::vector<Point> range_search(const Point& query, double radius) const = 0;
};

}  // namespace index
}

#endif
