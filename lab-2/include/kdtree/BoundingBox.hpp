#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include "Point.hpp"

namespace kdtree {

class BoundingBox {
  public:
    BoundingBox(const Point &lower_bound, const Point &upper_bound);

    bool contains(const Point &point) const;
    bool intersects(const BoundingBox &other) const;

    const Point &get_lower_bound() const;
    const Point &get_upper_bound() const;

  private:
    Point lower_bound_;
    Point upper_bound_;
};

} // namespace kdtree

#endif
