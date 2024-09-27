#include "inverted_index/Skiplists.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>

namespace inverted_index {

class SkiplistsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for testing
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
    }

    Skiplists skiplists_;
};

TEST_F(SkiplistsTest, AddAndRetrieveSkipPointers) {
    std::string term = "example";
    SkipPointer sp1 = {10, 100};
    SkipPointer sp2 = {20, 200};
    skiplists_.addSkipPointer(term, sp1);
    skiplists_.addSkipPointer(term, sp2);

    ASSERT_TRUE(skiplists_.hasSkipPointers(term));

    const auto& skips = skiplists_.getSkipPointers(term);
    ASSERT_EQ(skips.size(), 2);
    EXPECT_EQ(skips[0].doc_id, 10);
    EXPECT_EQ(skips[0].byte_offset, 100);
    EXPECT_EQ(skips[1].doc_id, 20);
    EXPECT_EQ(skips[1].byte_offset, 200);
}

TEST_F(SkiplistsTest, RetrieveNonexistentTerm) {
    std::string term = "nonexistent";
    ASSERT_FALSE(skiplists_.hasSkipPointers(term));

    const auto& skips = skiplists_.getSkipPointers(term);
    EXPECT_TRUE(skips.empty());
}

TEST_F(SkiplistsTest, ClearSkippointers) {
    std::string term = "example";
    SkipPointer sp = {10, 100};
    skiplists_.addSkipPointer(term, sp);

    ASSERT_TRUE(skiplists_.hasSkipPointers(term));

    skiplists_.clear();

    ASSERT_FALSE(skiplists_.hasSkipPointers(term));
}

TEST_F(SkiplistsTest, BuildSkipPointersWithEmptyData) {
    std::string term = "empty";
    std::vector<uint8_t> compressed_data = {};
    skiplists_.buildSkipPointers(term, compressed_data);

    ASSERT_FALSE(skiplists_.hasSkipPointers(term));
}

TEST_F(SkiplistsTest, BuildSkipPointersWithValidData) {
    std::string term = "valid";
    // Simulate a compressed posting list with two blocks:
    // Block 1: p=3, gap_count=2, gaps=[1,2] -> encoded as [3,2, ...]
    // Block 2: p=3, gap_count=2, gaps=[3,4] -> encoded as [3,2, ...]
    std::vector<uint8_t> compressed_data = {
        3, 2, // Block 1: p=3, gap_count=2
        // Gaps: 1 and 2, packed with p=3 bits
        0x11, // 0b00101000, // gap1=1 (001), gap2=2 (010)
        3, 2, // Block 2: p=3, gap_count=2
        // Gaps: 3 and 4, packed with p=3 bits
        0x23, // 0b01110000  // gap3=3 (011), gap4=4 (100)
    };

    EXPECT_NO_THROW({
        skiplists_.buildSkipPointers(term, compressed_data);
    });

    ASSERT_TRUE(skiplists_.hasSkipPointers(term));

    const auto& skips = skiplists_.getSkipPointers(term);
    ASSERT_EQ(skips.size(), 2);
    EXPECT_EQ(skips[0].doc_id, 1);   // Starting doc_id of Block 1
    EXPECT_EQ(skips[0].byte_offset, 0); // Byte offset of Block 1
    EXPECT_EQ(skips[1].doc_id, 6);   // Starting doc_id of Block 2
    EXPECT_EQ(skips[1].byte_offset, 3); // Byte offset of Block 2
}

TEST_F(SkiplistsTest, BuildSkipPointersWithCorruptedData) {
    std::string term = "corrupted";
    // Corrupted compressed data: p=2, gap_count=40, but only 1 byte provided instead of 8 bytes needed
    std::vector<uint8_t> compressed_data = {
        3, 2, // Block 1: p=3, gap_count=2
        0xAA, // Gaps [1,2] packed as 0b10101010 -> 0xAA
        2, 40  // Block 2: p=2, gap_count=40 (requires 10 bytes, but none provided)
        // Missing gap data for Block 2
    };

    EXPECT_THROW({
        skiplists_.buildSkipPointers(term, compressed_data);
    }, std::invalid_argument);
}

} // namespace inverted_index

