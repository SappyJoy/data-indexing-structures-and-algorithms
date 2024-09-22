#include "ehash/ExtensibleHashing.hpp"
#include "TestMessage.pb.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <memory>

namespace ehash {

using namespace ehash::proto;

// Temporary test directory for buckets
const std::string TEST_DIR = "test_buckets";

class ExtensibleHashingTest : public ::testing::Test {
  protected:
    void SetUp() override {
        try {
            if (std::filesystem::exists(TEST_DIR)) {
                std::filesystem::remove_all(TEST_DIR);
            }
            std::filesystem::create_directory(TEST_DIR);
        } catch (const std::exception &e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
        }
    }

    void TearDown() override {
        try {
            std::filesystem::remove_all(TEST_DIR);
        } catch (const std::exception &e) {
            std::cerr << "Error removing directory: " << e.what() << std::endl;
        }
    }
};

// Helper function to create a TestMessage with a specific ID
std::unique_ptr<TestMessage> createTestMessage(int id) {
    std::unique_ptr<TestMessage> message = std::make_unique<TestMessage>();
    message->set_id(id);
    return message;
}

// Test: Add a single entry and retrieve it
TEST_F(ExtensibleHashingTest, AddSingleEntry) {
    ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 4096);

    // Create a new TestMessage
    auto entry = createTestMessage(1);
    size_t hashValue = hashTable.addEntry(std::move(entry));

    // Retrieve the entries for the same hash
    const auto &entries = hashTable.getEntries(hashValue);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0]->id(), 1);
}

// Test: Adding and retrieving entries
TEST_F(ExtensibleHashingTest, AddAndRetrieveEntries) {
    // Initialize ExtensibleHashing with bucket size 1024 bytes
    ehash::ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 1024, 3);

    // Add entries
    size_t hash_1 = hashTable.addEntry(createTestMessage(1));
    size_t hash_2 = hashTable.addEntry(createTestMessage(2));
    size_t hash_3 = hashTable.addEntry(createTestMessage(3));

    // hashTable.print();

    // Retrieve entries by their key (ID as a string)
    const auto &entries = hashTable.getEntries(hash_1);
    EXPECT_EQ(entries[0]->id(), 1);

    const auto &entries2 = hashTable.getEntries(hash_2);
    EXPECT_EQ(entries2[0]->id(), 2);

    const auto &entries3 = hashTable.getEntries(hash_3);
    EXPECT_EQ(entries3[0]->id(), 3);
}

// Test: Add entries that cause a bucket split
TEST_F(ExtensibleHashingTest, BucketSplit) {
    // Create a small bucket size to force a split early
    ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 1024, 1);

    size_t oldBucketCount = hashTable.bucketCount();

    // std::cout << "Before adding entries" << std::endl;
    // hashTable.print();

    // Add entries until bucket splits
    std::vector<size_t> hashes;
    for (int i = 1; i <= 10000; ++i) {
        auto entry = createTestMessage(i);
        size_t hashValue = hashTable.addEntry(std::move(entry));
        hashes.push_back(hashValue);
    }

    // std::cout << "After adding entries" << std::endl;
    // hashTable.print();

    // Verify that all entries are still retrievable after split
    for (size_t i = 0; i < hashes.size(); ++i) {
        const auto entry = hashTable.getEntry(hashes[i]);
        ASSERT_TRUE(entry.has_value());
        EXPECT_EQ(entry.value()->id(), i + 1);
    }

    // Check if the global depth increased after split
    // Depending on implementation, global depth should have increased after the split
    // hashTable.print(); // Debug output to check directory structure and bucket contents

    EXPECT_GT(hashTable.bucketCount(), oldBucketCount);
}

// // Test: Rehashing when global depth is exceeded
// TEST_F(ExtensibleHashingTest, GlobalDepthDoubling) {
//     // Set a very small bucket size to quickly force global depth increase
//     ehash::ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 64); // Very small to force early rehash
//
//     // Add entries until global depth needs to be doubled
//     for (int i = 1; i <= 10; ++i) {
//         hashTable.addEntry(createTestMessage(i));
//     }
//
//     // Test that all entries are still accessible
//     for (int i = 1; i <= 10; ++i) {
//         const auto &entries = hashTable.getEntries(std::to_string(i));
//         ASSERT_EQ(entries.size(), 1);
//         EXPECT_EQ(entries[0]->id(), i);
//     }
// }
//
// // Test: Handle duplicate keys (overwrite)
// TEST_F(ExtensibleHashingTest, HandleDuplicateKeys) {
//     ehash::ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 1024);
//
//     // Add an entry
//     hashTable.addEntry(createTestMessage(1));
//
//     // Add another entry with the same key (this should overwrite)
//     auto newMessage = createTestMessage(1);
//     newMessage->set_id(100); // Change value
//     hashTable.addEntry(std::move(newMessage));
//
//     // Verify that the entry has been updated
//     const auto &entries = hashTable.getEntries("1");
//     ASSERT_EQ(entries.size(), 1);
//     EXPECT_EQ(entries[0]->id(), 100); // New value should be present
// }
//
// // Test: Handle empty entries
// TEST_F(ExtensibleHashingTest, HandleEmptyEntries) {
//     ehash::ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 1024);
//
//     // Attempt to add an empty entry
//     std::unique_ptr<TestMessage> emptyMessage = std::make_unique<TestMessage>();
//     emptyMessage->set_id(0); // Default key
//
//     ASSERT_NO_THROW(hashTable.addEntry(std::move(emptyMessage)));
//
//     // Check that the empty entry is retrievable
//     const auto &entries = hashTable.getEntries("0");
//     ASSERT_EQ(entries.size(), 1);
//     EXPECT_EQ(entries[0]->id(), 0); // Default empty entry should be retrievable
// }

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace ehash
