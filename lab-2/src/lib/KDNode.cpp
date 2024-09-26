#include "kdtree/KDNode.hpp"

namespace kdtree {

KDNode::KDNode(const Point &point, std::size_t axis) : point_(point), axis_(axis), left_(nullptr), right_(nullptr) {}

const Point &KDNode::point() const { return point_; }

std::size_t KDNode::axis() const { return axis_; }

std::shared_ptr<KDNode> KDNode::left() const { return left_; }

std::shared_ptr<KDNode> KDNode::right() const { return right_; }

void KDNode::set_left(std::shared_ptr<KDNode> left) { left_ = left; }

void KDNode::set_right(std::shared_ptr<KDNode> right) { right_ = right; }

} // namespace kdtree
