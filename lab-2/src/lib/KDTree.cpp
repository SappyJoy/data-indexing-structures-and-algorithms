#include "kdtree/KDTree.hpp"
#include "kdtree/KDNode.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace kdtree {

// Constructor: Default
KDTree::KDTree() : root_(nullptr), dimension_(0) {}

// Constructor: Build tree from points
KDTree::KDTree(const std::vector<Point> &points) : root_(nullptr), dimension_(0) { build(points); }

// Build the KD-tree from a set of points
void KDTree::build(const std::vector<Point> &points) {
    if (points.empty()) {
        throw std::invalid_argument("Point set is empty.");
    }
    dimension_ = points[0].dimension();
    // Verify all points have the same dimension
    for (const auto &point : points) {
        if (point.dimension() != dimension_) {
            throw std::invalid_argument("All points must have the same dimension.");
        }
    }
    std::vector<Point> points_copy = points;
    root_ = build_tree(points_copy.begin(), points_copy.end(), 0);
}

// Insert a single point into the KD-tree
void KDTree::insert(const Point &point) {
    if (dimension_ == 0) {
        dimension_ = point.dimension();
    } else if (point.dimension() != dimension_) {
        throw std::invalid_argument("Point dimensionality does not match KD-tree.");
    }
    root_ = insert_point(root_, point, 0);
}

// k-Nearest Neighbors search
std::vector<Point> KDTree::nearest_neighbors(const Point &query, size_t k) const {
    if (k == 0) {
        return {};
    }
    if (!root_) {
        return {};
    }

    // Create the priority queue with the comparator struct
    std::priority_queue<std::pair<double, Point>, std::vector<std::pair<double, Point>>, CompareDistance> best_points;

    // Perform the recursive search
    nearest_neighbors(root_, query, k, best_points);

    // Extract points from the heap
    std::vector<Point> result;
    result.reserve(best_points.size());
    while (!best_points.empty()) {
        result.emplace_back(std::move(best_points.top().second));
        best_points.pop();
    }

    // Reverse to have closest points first
    std::reverse(result.begin(), result.end());

    return result;
}

// Range search
std::vector<Point> KDTree::range_search(const Point &query, double radius) const {
    if (!root_) {
        return {};
    }
    std::vector<Point> results;
    range_search(root_, query, radius, results);
    return results;
}

// Private helper function to build the tree recursively
std::shared_ptr<KDNode> KDTree::build_tree(std::vector<Point>::iterator begin, std::vector<Point>::iterator end,
                                           size_t depth) {
    if (begin >= end) {
        return nullptr;
    }
    size_t axis = depth % dimension_;
    // Sort points along the current axis
    auto comparator = [axis](const Point &a, const Point &b) { return a[axis] < b[axis]; };
    auto mid = begin + (end - begin) / 2;
    std::nth_element(begin, mid, end, comparator);
    // Create node and construct subtrees
    auto node = std::make_shared<KDNode>(*mid, axis);
    node->set_left(build_tree(begin, mid, depth + 1));
    node->set_right(build_tree(mid + 1, end, depth + 1));
    return node;
}

// Private helper function to insert a point into the tree
std::shared_ptr<KDNode> KDTree::insert_point(std::shared_ptr<KDNode> node, const Point &point, size_t depth) {
    if (!node) {
        size_t axis = depth % dimension_;
        return std::make_shared<KDNode>(point, axis);
    }
    size_t axis = node->axis();
    if (point[axis] < node->point()[axis]) {
        node->set_left(insert_point(node->left(), point, depth + 1));
    } else {
        node->set_right(insert_point(node->right(), point, depth + 1));
    }
    return node;
}

// Private helper function for k-NN search
void KDTree::nearest_neighbors(const std::shared_ptr<KDNode> &node, const Point &query, size_t k,
                               std::priority_queue<std::pair<double, Point>, std::vector<std::pair<double, Point>>,
                                                   CompareDistance> &best_points) const {
    if (!node) {
        return;
    }

    // Compute distance between query and current node
    double dist = distance(query, node->point());

    if (best_points.size() < k) {
        best_points.emplace(dist, node->point());
    } else if (dist < best_points.top().first) {
        best_points.pop();
        best_points.emplace(dist, node->point());
    }

    // Decide which subtree to search first
    size_t axis = node->axis();
    bool go_left = query[axis] < node->point()[axis];

    const auto &first_branch = go_left ? node->left() : node->right();
    const auto &second_branch = go_left ? node->right() : node->left();

    // Search the first branch
    nearest_neighbors(first_branch, query, k, best_points);

    // Determine if we need to search the opposite branch
    if (best_points.size() < k || std::abs(query[axis] - node->point()[axis]) < best_points.top().first) {
        nearest_neighbors(second_branch, query, k, best_points);
    }
}

// Private helper function for range search
void KDTree::range_search(const std::shared_ptr<KDNode> &node, const Point &query, double radius,
                          std::vector<Point> &results) const {
    if (!node) {
        return;
    }
    // Compute distance between query and current node
    double dist = distance(query, node->point());
    if (dist <= radius) {
        results.push_back(node->point());
    }
    // Decide whether to search left, right, or both subtrees
    size_t axis = node->axis();
    if (query[axis] - radius <= node->point()[axis]) {
        range_search(node->left(), query, radius, results);
    }
    if (query[axis] + radius >= node->point()[axis]) {
        range_search(node->right(), query, radius, results);
    }
}

// Euclidean distance between two points
double KDTree::distance(const Point &a, const Point &b) const {
    double dist = 0.0;
    for (size_t i = 0; i < dimension_; ++i) {
        double diff = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        dist += diff * diff;
    }
    return std::sqrt(dist);
}

} // namespace kdtree
