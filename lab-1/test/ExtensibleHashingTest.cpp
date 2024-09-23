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

    // Add entries until bucket splits
    std::vector<size_t> hashes;
    for (int i = 1; i <= 10000; ++i) {
        auto entry = createTestMessage(i);
        size_t hashValue = hashTable.addEntry(std::move(entry));
        // std::cout << "Added entry with hash: " << hashValue << std::endl;
        // hashTable.print();
        hashes.push_back(hashValue);
    }

    // Verify that all entries are still retrievable after split
    for (size_t i = 0; i < hashes.size(); ++i) {
        const auto entry = hashTable.getEntry(hashes[i]);
        ASSERT_TRUE(entry.has_value());
        EXPECT_EQ(entry.value()->id(), i + 1);
    }

    EXPECT_GT(hashTable.bucketCount(), oldBucketCount);
}

// Test: Adding an entry with a duplicate key should update the existing entry
TEST_F(ExtensibleHashingTest, UpdateDuplicateKey) {
    ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 4096);

    // Create and add an entry
    auto entry = createTestMessage(1);
    size_t hashValue = hashTable.addEntry(std::move(entry));

    // Check that the entry was added correctly
    const auto &entries = hashTable.getEntries(hashValue);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_EQ(entries[0]->id(), 1);

    // Create a new entry with the same key but different data
    auto updatedEntry = createTestMessage(1); // Same ID
    hashTable.addEntry(std::move(updatedEntry));

    // Check that the entry was updated
    const auto &updatedEntries = hashTable.getEntries(hashValue);
    ASSERT_EQ(updatedEntries.size(), 1); // Still only 1 entry
    EXPECT_EQ(updatedEntries[0]->id(), 1);
}

// Test: Adding a duplicate key after a bucket split should update the entry in the correct bucket
TEST_F(ExtensibleHashingTest, UpdateDuplicateKeyAfterSplit) {
    // Create a small bucket size to force a split early
    ExtensibleHashing<TestMessage> hashTable(TEST_DIR, 1024, 1);

    // Add enough entries to cause a split
    std::vector<size_t> hashes;
    for (int i = 1; i <= 2000; ++i) {
        auto entry = createTestMessage(i);
        size_t hashValue = hashTable.addEntry(std::move(entry));
        hashes.push_back(hashValue);
    }

    // Pick an entry to update after the split
    auto hashToUpdate = hashes[1000];            // Pick the entry with ID 51
    auto updatedEntry = createTestMessage(1001); // Same ID as the 51st entry

    // Update the entry
    hashTable.addEntry(std::move(updatedEntry));

    // Retrieve the updated entry and verify the data
    const auto &entriesAfterUpdate = hashTable.getEntries(hashToUpdate);

    std::size_t duplicates = 0;
    for (const auto &entry : entriesAfterUpdate) {
        if (entry->id() == 1001) {
            duplicates++;
        }
    }
    EXPECT_EQ(duplicates, 1);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

} // namespace ehash
