#include "kdtree/Point.hpp"
#include "gtest/gtest.h"
#include <vector>

namespace kdtree {
namespace tests {

// Test Fixture for Point
class PointTest : public ::testing::Test {
  protected:
    // You can remove any or all of the following functions if its body is empty.

    PointTest() {
        // You can do set-up work for each test here.
    }

    ~PointTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // Objects declared here can be used by all tests in the test suite.
};

// Test Constructor with Coordinates
TEST_F(PointTest, ConstructorWithCoordinates) {
    std::vector<float> coords = {1.0f, 2.0f, 3.0f};
    Point p(coords);
    EXPECT_EQ(p.dimension(), 3);
    EXPECT_FLOAT_EQ(p[0], 1.0f);
    EXPECT_FLOAT_EQ(p[1], 2.0f);
    EXPECT_FLOAT_EQ(p[2], 3.0f);
}

// Test Operator[] for Accessing Elements
TEST_F(PointTest, AccessElements) {
    std::vector<float> coords = {4.5f, 5.5f};
    Point p(coords);
    EXPECT_FLOAT_EQ(p[0], 4.5f);
    EXPECT_FLOAT_EQ(p[1], 5.5f);
}

// Test Operator[] for Modifying Elements
TEST_F(PointTest, ModifyElements) {
    std::vector<float> coords = {7.0f, 8.0f, 9.0f};
    Point p(coords);
    p[0] = 10.0f;
    p[2] = 12.0f;
    EXPECT_FLOAT_EQ(p[0], 10.0f);
    EXPECT_FLOAT_EQ(p[1], 8.0f);
    EXPECT_FLOAT_EQ(p[2], 12.0f);
}

// Test Dimension Consistency
TEST_F(PointTest, DimensionConsistency) {
    std::vector<float> coords1 = {1.0f, 2.0f};
    std::vector<float> coords2 = {3.0f, 4.0f, 5.0f};
    Point p1(coords1);
    Point p2(coords2);
    EXPECT_EQ(p1.dimension(), 2);
    EXPECT_EQ(p2.dimension(), 3);
}

// Test Empty Coordinates
TEST_F(PointTest, EmptyCoordinates) {
    std::vector<float> coords;
    Point p(coords);
    EXPECT_EQ(p.dimension(), 0);
}

// Test Copy Constructor
TEST_F(PointTest, CopyConstructor) {
    std::vector<float> coords = {6.0f, 7.0f};
    Point p1(coords);
    Point p2 = p1; // Copy constructor
    EXPECT_EQ(p2.dimension(), 2);
    EXPECT_FLOAT_EQ(p2[0], 6.0f);
    EXPECT_FLOAT_EQ(p2[1], 7.0f);
}

// Test Assignment Operator
TEST_F(PointTest, AssignmentOperator) {
    std::vector<float> coords1 = {2.2f, 3.3f};
    std::vector<float> coords2 = {4.4f, 5.5f, 6.6f};
    Point p1(coords1);
    Point p2(coords2);
    p2 = p1; // Assignment operator
    EXPECT_EQ(p2.dimension(), 2);
    EXPECT_FLOAT_EQ(p2[0], 2.2f);
    EXPECT_FLOAT_EQ(p2[1], 3.3f);
}

} // namespace tests
} // namespace kdtree
