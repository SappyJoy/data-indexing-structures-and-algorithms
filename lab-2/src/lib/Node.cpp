#include "kdtree/Node.hpp"

namespace kdtree {

Node::Node(const Point &point) : left(nullptr), right(nullptr), point_(point) {}

const Point &Node::get_point() const { return point_; }

} // namespace kdtree
