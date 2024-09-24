#ifndef NODE_HPP
#define NODE_HPP

#include "Point.hpp"

namespace kdtree {

class Node {
  public:
    Node(const Point &point);
    Node *left;
    Node *right;
    const Point &get_point() const;

  private:
    Point point_;
};

} // namespace kdtree

#endif
