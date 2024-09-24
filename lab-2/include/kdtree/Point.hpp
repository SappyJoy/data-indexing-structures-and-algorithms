#ifndef POINT_HPP
#define POINT_HPP

#include <cstddef>
#include <vector>

namespace kdtree {

class Point {
  public:
    Point(const std::vector<double> &coordinates);
    double get_coordinate(std::size_t index) const;
    size_t dimension() const;
    // Euclidean distance
    double distance_to(const Point &other) const;

  private:
    std::vector<double> coordinates_;
};

} // namespace kdtree

#endif
