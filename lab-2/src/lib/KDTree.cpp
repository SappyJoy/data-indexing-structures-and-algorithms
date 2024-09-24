#include "kdtree/KDTree.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace kdtree {

// Конструктор KD-дерева
KDTree::KDTree(size_t dimension) : root_(nullptr), dimension_(dimension) {
    if (dimension == 0) {
        throw std::invalid_argument("Dimension must be greater than zero");
    }
}

// Вставка точки в дерево
void KDTree::insert(const Point &point) {
    if (point.dimension() != dimension_) {
        throw std::invalid_argument("Dimension of the point must match the dimension of the tree");
    }
    root_.reset(insert_recursive(root_.release(), point, 0));
}

// Вспомогательная рекурсивная функция для вставки
Node *KDTree::insert_recursive(Node *node, const Point &point, size_t depth) {
    if (node == nullptr) {
        return new Node(point);
    }

    size_t axis = depth % dimension_; // Определяем ось по глубине

    if (point.get_coordinate(axis) < node->get_point().get_coordinate(axis)) {
        node->left = insert_recursive(node->left, point, depth + 1);
    } else {
        node->right = insert_recursive(node->right, point, depth + 1);
    }

    return node;
}

// Поиск ближайшего соседа
Point KDTree::nearest_neighbor(const Point &target) const {
    if (!root_) {
        throw std::runtime_error("Tree is empty");
    }

    double best_distance = std::numeric_limits<double>::max();
    Node *best = nearest_neighbor_recursive(root_.get(), target, 0, nullptr, best_distance);

    if (!best) {
        throw std::runtime_error("Failed to find nearest neighbor");
    }
    return best->get_point();
}

// Вспомогательная рекурсивная функция для поиска ближайшего соседа
Node *KDTree::nearest_neighbor_recursive(Node *node, const Point &target, size_t depth, Node *best,
                                         double &best_distance) const {
    if (!node)
        return best;

    double distance_to_node = distance(node->get_point(), target);
    if (distance_to_node < best_distance) {
        best_distance = distance_to_node;
        best = node;
    }

    size_t axis = depth % dimension_;
    double diff = target.get_coordinate(axis) - node->get_point().get_coordinate(axis);

    Node *near_subtree = (diff < 0) ? node->left : node->right;
    Node *far_subtree = (diff < 0) ? node->right : node->left;

    best = nearest_neighbor_recursive(near_subtree, target, depth + 1, best, best_distance);

    if (std::fabs(diff) < best_distance) {
        best = nearest_neighbor_recursive(far_subtree, target, depth + 1, best, best_distance);
    }

    return best;
}

// Поиск точек в пределах заданной области (BoundingBox)
std::vector<Point> KDTree::range_search(const BoundingBox &bounds) const {
    std::vector<Point> results;
    if (root_) {
        range_search_recursive(root_.get(), bounds, results, 0);
    }
    return results;
}

// Вспомогательная рекурсивная функция для поиска по диапазону
void KDTree::range_search_recursive(Node *node, const BoundingBox &bounds, std::vector<Point> &results,
                                    size_t depth) const {
    if (!node)
        return;

    if (bounds.contains(node->get_point())) {
        results.push_back(node->get_point());
    }

    size_t axis = depth % dimension_;
    if (bounds.get_lower_bound().get_coordinate(axis) <= node->get_point().get_coordinate(axis)) {
        range_search_recursive(node->left, bounds, results, depth + 1);
    }
    if (bounds.get_upper_bound().get_coordinate(axis) >= node->get_point().get_coordinate(axis)) {
        range_search_recursive(node->right, bounds, results, depth + 1);
    }
}

// Вычисление евклидова расстояния между двумя точками
double KDTree::distance(const Point &a, const Point &b) const { return a.distance_to(b); }

} // namespace kdtree
