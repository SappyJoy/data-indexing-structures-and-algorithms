#ifndef KDTREE_HPP
#define KDTREE_HPP

#include "kdtree/BoundingBox.hpp"
#include "kdtree/Node.hpp"
#include "kdtree/Point.hpp"
#include <cstddef>
#include <memory>

namespace kdtree {

class KDTree {
  public:
    KDTree(std::size_t dimension);

    // Вставка точки
    void insert(const Point &point);

    // Поиск ближайшего соседа
    Point nearest_neighbor(const Point &target) const;

    // Поиск точек в пределах заданной области
    std::vector<Point> range_search(const BoundingBox &bounds) const;

  private:
    Node *insert_recursive(Node *node, const Point &point, size_t depth);
    Node *nearest_neighbor_recursive(Node *node, const Point &target, size_t depth, Node *best,
                                     double &best_distance) const;
    void range_search_recursive(Node *node, const BoundingBox &bounds, std::vector<Point> &results, size_t depth) const;
    double distance(const Point &a, const Point &b) const;

    std::unique_ptr<Node> root_; // Указатель на корень дерева
    size_t dimension_;           // Размерность пространства
};

} // namespace kdtree

#endif
