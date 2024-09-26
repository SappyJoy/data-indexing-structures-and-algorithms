#include "kdtree/indexes/FAISSIndex.hpp"
#include "kdtree/Point.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace kdtree {
namespace tests {

// Test Fixture for FAISSIndex
class FAISSIndexTest : public ::testing::Test {
  protected:
    // You can remove any or all of the following functions if its body is empty.

    FAISSIndexTest() {
        // You can do set-up work for each test here.
    }

    ~FAISSIndexTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // Objects declared here can be used by all tests in the test suite.
    std::unique_ptr<FAISSIndex> index_;
};

// Test Building the FAISSIndex with a set of points
TEST_F(FAISSIndexTest, BuildWithPoints) {
    std::vector<Point> points = {Point({2.0f, 3.0f}), Point({5.0f, 4.0f}), Point({9.0f, 6.0f}),
                                 Point({4.0f, 7.0f}), Point({8.0f, 1.0f}), Point({7.0f, 2.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    // Perform a simple nearest neighbor search to verify
    Point query({5.0f, 5.0f});
    size_t k = 2;
    std::vector<Point> neighbors = index_->nearest_neighbors(query, k);

    ASSERT_EQ(neighbors.size(), k);
    EXPECT_FLOAT_EQ(neighbors[0][0], 5.0f);
    EXPECT_FLOAT_EQ(neighbors[0][1], 4.0f);
    EXPECT_FLOAT_EQ(neighbors[1][0], 4.0f);
    EXPECT_FLOAT_EQ(neighbors[1][1], 7.0f);
}

// Test Inserting Points into FAISSIndex
TEST_F(FAISSIndexTest, InsertPoints) {
    std::vector<Point> initial_points = {Point({1.0f, 2.0f}), Point({3.0f, 4.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(initial_points);

    // Insert a new point
    Point new_point({5.0f, 6.0f});
    index_->insert(new_point);

    // Perform a nearest neighbor search
    Point query({5.0f, 5.0f});
    size_t k = 3;
    std::vector<Point> neighbors = index_->nearest_neighbors(query, k);

    ASSERT_EQ(neighbors.size(), 3);
    EXPECT_FLOAT_EQ(neighbors[0][0], 5.0f);
    EXPECT_FLOAT_EQ(neighbors[0][1], 6.0f);
    EXPECT_FLOAT_EQ(neighbors[1][0], 3.0f);
    EXPECT_FLOAT_EQ(neighbors[1][1], 4.0f);
    EXPECT_FLOAT_EQ(neighbors[2][0], 1.0f);
    EXPECT_FLOAT_EQ(neighbors[2][1], 2.0f);
}

// Test Nearest Neighbors Search in FAISSIndex
TEST_F(FAISSIndexTest, NearestNeighborsSearch) {
    std::vector<Point> points = {Point({2.0f, 3.0f}), Point({5.0f, 4.0f}), Point({9.0f, 6.0f}),
                                 Point({4.0f, 7.0f}), Point({8.0f, 1.0f}), Point({7.0f, 2.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    Point query({5.0f, 5.0f});
    size_t k = 2;
    std::vector<Point> neighbors = index_->nearest_neighbors(query, k);

    ASSERT_EQ(neighbors.size(), k);
    EXPECT_FLOAT_EQ(neighbors[0][0], 5.0f);
    EXPECT_FLOAT_EQ(neighbors[0][1], 4.0f);
    EXPECT_FLOAT_EQ(neighbors[1][0], 4.0f);
    EXPECT_FLOAT_EQ(neighbors[1][1], 7.0f);
}

// Test Range Search in FAISSIndex
TEST_F(FAISSIndexTest, RangeSearch) {
    std::vector<Point> points = {Point({2.0f, 3.0f}), Point({5.0f, 4.0f}), Point({9.0f, 6.0f}),
                                 Point({4.0f, 7.0f}), Point({8.0f, 1.0f}), Point({7.0f, 2.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    Point query({5.0f, 5.0f});
    double radius = 3.0;
    std::vector<Point> range_results = index_->range_search(query, radius);

    // Expected points within radius 3.0: {5.0,4.0}, {7.0,2.0}
    ASSERT_EQ(range_results.size(), 2);
    bool found_2_3 = false, found_5_4 = false, found_4_7 = false;
    for (const auto &p : range_results) {
        if (p.dimension() == 2) {
            if (p[0] == 2.0f && p[1] == 3.0f)
                found_2_3 = true;
            if (p[0] == 5.0f && p[1] == 4.0f)
                found_5_4 = true;
            if (p[0] == 4.0f && p[1] == 7.0f)
                found_4_7 = true;
        }
    }
    EXPECT_FALSE(found_2_3);
    EXPECT_TRUE(found_5_4);
    EXPECT_TRUE(found_4_7);
}

// Test Nearest Neighbors with k greater than number of points
TEST_F(FAISSIndexTest, NearestNeighborsKGreaterThanSize) {
    std::vector<Point> points = {Point({1.0f, 1.0f}), Point({2.0f, 2.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    Point query({1.5f, 1.5f});
    size_t k = 5;
    std::vector<Point> neighbors = index_->nearest_neighbors(query, k);

    // Should return only 2 neighbors
    ASSERT_EQ(neighbors.size(), 2);
    EXPECT_FLOAT_EQ(neighbors[0][0], 1.0f);
    EXPECT_FLOAT_EQ(neighbors[0][1], 1.0f);
    EXPECT_FLOAT_EQ(neighbors[1][0], 2.0f);
    EXPECT_FLOAT_EQ(neighbors[1][1], 2.0f);
}

// Test Range Search with No Results
TEST_F(FAISSIndexTest, RangeSearchNoResults) {
    std::vector<Point> points = {Point({10.0f, 10.0f}), Point({20.0f, 20.0f}), Point({30.0f, 30.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    Point query({0.0f, 0.0f});
    double radius = 5.0;
    std::vector<Point> range_results = index_->range_search(query, radius);

    EXPECT_TRUE(range_results.empty());
}

// Test Building FAISSIndex with Inconsistent Dimensions
TEST_F(FAISSIndexTest, BuildWithInconsistentDimensions) {
    std::vector<Point> points = {
        Point({1.0f, 2.0f}), Point({3.0f, 4.0f, 5.0f}) // 3D point in a 2D FAISSIndex
    };
    index_ = std::make_unique<FAISSIndex>();

    // Assuming FAISSIndex::build throws std::invalid_argument for dimension mismatch
    EXPECT_THROW(index_->build(points), std::invalid_argument);
}

// Test Inserting Point with Different Dimensions After Building
TEST_F(FAISSIndexTest, InsertWithDifferentDimensions) {
    std::vector<Point> points = {Point({1.0f, 2.0f}), Point({3.0f, 4.0f})};
    index_ = std::make_unique<FAISSIndex>();
    index_->build(points);

    std::vector<float> coords = {5.0f, 6.0f, 7.0f}; // 3D point
    Point new_point(coords);

    // Assuming FAISSIndex::insert throws std::invalid_argument for dimension mismatch
    EXPECT_THROW(index_->insert(new_point), std::invalid_argument);
}

// Test Nearest Neighbors Search with Empty FAISSIndex
TEST_F(FAISSIndexTest, NearestNeighborsEmptyIndex) {
    index_ = std::make_unique<FAISSIndex>();
    // Not building the index, leaving it empty

    Point query({1.0f, 1.0f});
    size_t k = 3;
    // Expect exception when calling nearest_neighbors
    EXPECT_THROW(index_->nearest_neighbors(query, k), std::runtime_error);
}

// Test Range Search with Empty FAISSIndex
TEST_F(FAISSIndexTest, RangeSearchEmptyIndex) {
    index_ = std::make_unique<FAISSIndex>();
    // Not building the index, leaving it empty

    Point query({1.0f, 1.0f});
    double radius = 10.0;
    // Expect exception when calling range_search
    EXPECT_THROW(index_->range_search(query, radius), std::runtime_error);
}

} // namespace tests
} // namespace kdtree
