#include "ExtensibleHashing.hpp"
#include "gtest/gtest.h"
#include <optional>

namespace ehash {

TEST(ExtensibleHashingTest, InsertAndSearch) {
    ExtensibleHashing eh(2);

    // Insert key-value pairs
    eh.insert(1, 10);
    eh.insert(2, 20);
    eh.insert(3, 30);

    // Search for keys
    EXPECT_EQ(eh.search(1), 10);
    EXPECT_EQ(eh.search(2), 20);
    EXPECT_EQ(eh.search(3), 30);
}

TEST(ExtensibleHashingTest, Remove) {
    ExtensibleHashing eh(2);

    // Insert key-value pairs
    eh.insert(1, 10);
    eh.insert(2, 20);
    eh.insert(3, 30);

    // Remove a key
    EXPECT_TRUE(eh.remove(2));

    // Search for keys
    EXPECT_EQ(eh.search(1), 10);
    EXPECT_EQ(eh.search(2), std::nullopt); // Key 2 should have been removed
    EXPECT_EQ(eh.search(3), 30);
}

TEST(ExtensibleHashingTest, SplitBucket) {
    ExtensibleHashing eh(2);

    // Insert key-value pairs that will cause a bucket split
    eh.insert(1, 10);
    eh.insert(2, 20);
    eh.insert(3, 30);
    eh.insert(4, 40);

    // Search for keys
    EXPECT_EQ(eh.search(1), 10);
    EXPECT_EQ(eh.search(2), 20);
    EXPECT_EQ(eh.search(3), 30);
    EXPECT_EQ(eh.search(4), 40);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace ehash
