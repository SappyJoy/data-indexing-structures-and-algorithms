#include "kdtree/KDNode.hpp"
#include "kdtree/Point.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <vector>

namespace kdtree {
namespace tests {

// Test Fixture for KDNode
class KDNodeTest : public ::testing::Test {
  protected:
    // You can remove any or all of the following functions if its body is empty.

    KDNodeTest() {
        // You can do set-up work for each test here.
    }

    ~KDNodeTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // Objects declared here can be used by all tests in the test suite.
};

// Test Constructor
TEST_F(KDNodeTest, Constructor) {
    std::vector<float> coords = {1.0f, 2.0f, 3.0f};
    Point p(coords);
    size_t axis = 1;
    auto node = std::make_shared<KDNode>(p, axis);
    EXPECT_EQ(node->axis(), axis);
    EXPECT_EQ(node->point().dimension(), 3);
    EXPECT_FLOAT_EQ(node->point()[0], 1.0f);
    EXPECT_FLOAT_EQ(node->point()[1], 2.0f);
    EXPECT_FLOAT_EQ(node->point()[2], 3.0f);
    EXPECT_EQ(node->left(), nullptr);
    EXPECT_EQ(node->right(), nullptr);
}

// Test Setting and Getting Left Child
TEST_F(KDNodeTest, SetGetLeftChild) {
    Point parent_point({5.0f, 5.0f});
    auto parent_node = std::make_shared<KDNode>(parent_point, 0);

    Point left_point({3.0f, 3.0f});
    auto left_node = std::make_shared<KDNode>(left_point, 1);

    parent_node->set_left(left_node);

    EXPECT_EQ(parent_node->left(), left_node);
    EXPECT_EQ(parent_node->right(), nullptr);
}

// Test Setting and Getting Right Child
TEST_F(KDNodeTest, SetGetRightChild) {
    Point parent_point({5.0f, 5.0f});
    auto parent_node = std::make_shared<KDNode>(parent_point, 0);

    Point right_point({7.0f, 7.0f});
    auto right_node = std::make_shared<KDNode>(right_point, 1);

    parent_node->set_right(right_node);

    EXPECT_EQ(parent_node->right(), right_node);
    EXPECT_EQ(parent_node->left(), nullptr);
}

// Test Setting Both Children
TEST_F(KDNodeTest, SetBothChildren) {
    Point parent_point({5.0f, 5.0f});
    auto parent_node = std::make_shared<KDNode>(parent_point, 0);

    Point left_point({3.0f, 3.0f});
    auto left_node = std::make_shared<KDNode>(left_point, 1);

    Point right_point({7.0f, 7.0f});
    auto right_node = std::make_shared<KDNode>(right_point, 1);

    parent_node->set_left(left_node);
    parent_node->set_right(right_node);

    EXPECT_EQ(parent_node->left(), left_node);
    EXPECT_EQ(parent_node->right(), right_node);
}

// Test Accessing Point Data
TEST_F(KDNodeTest, AccessPointData) {
    std::vector<float> coords = {4.5f, 6.7f};
    Point p(coords);
    size_t axis = 1;
    KDNode node(p, axis);

    Point retrieved_point = node.point();
    EXPECT_EQ(retrieved_point.dimension(), 2);
    EXPECT_FLOAT_EQ(retrieved_point[0], 4.5f);
    EXPECT_FLOAT_EQ(retrieved_point[1], 6.7f);
}

// Test Axis Retrieval
TEST_F(KDNodeTest, RetrieveAxis) {
    Point p({2.0f, 3.0f});
    size_t axis = 0;
    KDNode node(p, axis);
    EXPECT_EQ(node.axis(), axis);
}

// Test Overwriting Children
TEST_F(KDNodeTest, OverwriteChildren) {
    Point parent_point({5.0f, 5.0f});
    auto parent_node = std::make_shared<KDNode>(parent_point, 0);

    Point first_left_point({3.0f, 3.0f});
    auto first_left = std::make_shared<KDNode>(first_left_point, 1);
    parent_node->set_left(first_left);

    Point second_left_point({2.0f, 2.0f});
    auto second_left = std::make_shared<KDNode>(second_left_point, 1);
    parent_node->set_left(second_left); // Overwrite left child

    EXPECT_EQ(parent_node->left(), second_left);
    EXPECT_EQ(parent_node->right(), nullptr);
}

} // namespace tests
} // namespace kdtree
