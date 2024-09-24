#include "kdtree/KDTree.hpp"
#include "kdtree/Point.hpp"
#include "gtest/gtest.h"
#include <vector>

namespace kdtree {

// Вспомогательная функция для создания тестовых данных
Point create_point(std::initializer_list<double> coords) { return Point(std::vector<double>(coords)); }

// Тест для вставки точек в KDTree
TEST(KDTreeTest, InsertPoints) {
    KDTree tree(2); // Дерево размерности 2

    Point p1 = create_point({1.0, 2.0});
    Point p2 = create_point({3.0, 6.0});
    Point p3 = create_point({5.0, 1.0});

    ASSERT_NO_THROW(tree.insert(p1));
    ASSERT_NO_THROW(tree.insert(p2));
    ASSERT_NO_THROW(tree.insert(p3));
}

// Тест для поиска ближайшего соседа
TEST(KDTreeTest, NearestNeighbor) {
    KDTree tree(2); // Дерево размерности 2

    Point p1 = create_point({1.0, 2.0});
    Point p2 = create_point({3.0, 6.0});
    Point p3 = create_point({5.0, 1.0});
    tree.insert(p1);
    tree.insert(p2);
    tree.insert(p3);

    // Ищем ближайшего соседа к точке (2.0, 3.0)
    Point target = create_point({2.0, 3.0});
    Point nearest = tree.nearest_neighbor(target);

    // Ожидаем, что ближайший сосед — это точка p1
    EXPECT_EQ(nearest.get_coordinate(0), p1.get_coordinate(0));
    EXPECT_EQ(nearest.get_coordinate(1), p1.get_coordinate(1));
}

// Тест для проверки исключения при поиске в пустом дереве
TEST(KDTreeTest, NearestNeighborEmptyTree) {
    KDTree tree(2); // Дерево размерности 2
    Point target = create_point({2.0, 3.0});

    // Ожидаем, что поиск ближайшего соседа в пустом дереве вызовет исключение
    EXPECT_THROW(tree.nearest_neighbor(target), std::runtime_error);
}

// Тест для поиска по диапазону (BoundingBox)
TEST(KDTreeTest, RangeSearch) {
    KDTree tree(2); // Дерево размерности 2

    Point p1 = create_point({1.0, 2.0});
    Point p2 = create_point({3.0, 3.0});
    Point p3 = create_point({5.0, 1.0});
    tree.insert(p1);
    tree.insert(p2);
    tree.insert(p3);

    // Ограничивающая область от (0, 0) до (4, 4)
    BoundingBox bounds(create_point({0.0, 0.0}), create_point({4.0, 4.0}));
    std::vector<Point> results = tree.range_search(bounds);

    // Ожидаем, что будут найдены p1 и p2
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].get_coordinate(0), p1.get_coordinate(0));
    EXPECT_EQ(results[0].get_coordinate(1), p1.get_coordinate(1));
    EXPECT_EQ(results[1].get_coordinate(0), p2.get_coordinate(0));
    EXPECT_EQ(results[1].get_coordinate(1), p2.get_coordinate(1));
}

// Тест на проверку размерности дерева
TEST(KDTreeTest, InvalidPointDimension) {
    KDTree tree(3); // Дерево размерности 3
    Point p1 = create_point({1.0, 2.0, 3.0});
    Point p2 = create_point({4.0, 5.0}); // Размерность не совпадает

    // Ожидаем, что вставка точки с неправильной размерностью вызовет исключение
    ASSERT_NO_THROW(tree.insert(p1));
    EXPECT_THROW(tree.insert(p2), std::invalid_argument);
}

// Тест на проверку пересечения с границами (BoundingBox)
TEST(KDTreeTest, RangeSearchEmptyResults) {
    KDTree tree(2); // Дерево размерности 2

    Point p1 = create_point({1.0, 2.0});
    Point p2 = create_point({3.0, 6.0});
    tree.insert(p1);
    tree.insert(p2);

    // Ограничивающая область от (7, 7) до (10, 10), которая не пересекается с точками
    BoundingBox bounds(create_point({7.0, 7.0}), create_point({10.0, 10.0}));
    std::vector<Point> results = tree.range_search(bounds);

    // Ожидаем, что не будет найдено ни одной точки
    EXPECT_TRUE(results.empty());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace kdtree
