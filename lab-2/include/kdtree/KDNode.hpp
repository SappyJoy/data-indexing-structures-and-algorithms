#ifndef KDNODE_HPP
#define KDNODE_HPP

#include "kdtree/Point.hpp"
#include <memory>

namespace kdtree {

class KDNode {
  public:
    // Constructors
    KDNode(const Point &point, size_t axis);

    // Accessors
    const Point &point() const;
    size_t axis() const;
    std::shared_ptr<KDNode> left() const;
    std::shared_ptr<KDNode> right() const;

    // Modifiers
    void set_left(std::shared_ptr<KDNode> left);
    void set_right(std::shared_ptr<KDNode> right);

  private:
    Point point_;
    size_t axis_;
    std::shared_ptr<KDNode> left_;
    std::shared_ptr<KDNode> right_;
};

} // namespace kdtree

#endif
