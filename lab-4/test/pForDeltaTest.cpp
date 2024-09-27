#include "inverted_index/pForDelta.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>

namespace inverted_index {

class PForDeltaTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Initialize logger for testing
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
    }
};

TEST_F(PForDeltaTest, EncodeDecodeEmptyList) {
    std::vector<int> doc_ids = {};
    std::vector<uint8_t> compressed = PForDelta::encode(doc_ids);
    std::vector<int> decoded = PForDelta::decode(compressed);
    EXPECT_EQ(decoded, doc_ids);
}

TEST_F(PForDeltaTest, EncodeDecodeSingleElement) {
    std::vector<int> doc_ids = {100};
    std::vector<uint8_t> compressed = PForDelta::encode(doc_ids);
    std::vector<int> decoded = PForDelta::decode(compressed);
    EXPECT_EQ(decoded, doc_ids);
}

TEST_F(PForDeltaTest, EncodeDecodeMultipleElements) {
    std::vector<int> doc_ids = {1, 3, 7, 15, 31, 63, 127};
    std::vector<uint8_t> compressed = PForDelta::encode(doc_ids);
    std::vector<int> decoded = PForDelta::decode(compressed);
    EXPECT_EQ(decoded, doc_ids);
}

TEST_F(PForDeltaTest, EncodeDecodeWithLargeGaps) {
    std::vector<int> doc_ids = {10, 1000, 100000, 10000000};
    std::vector<uint8_t> compressed = PForDelta::encode(doc_ids);
    std::vector<int> decoded = PForDelta::decode(compressed);
    EXPECT_EQ(decoded, doc_ids);
}

TEST_F(PForDeltaTest, EncodeDecodeConsecutiveIds) {
    std::vector<int> doc_ids;
    for (int i = 1; i <= 1000; ++i) {
        doc_ids.push_back(i);
    }
    std::vector<uint8_t> compressed = PForDelta::encode(doc_ids);
    std::vector<int> decoded = PForDelta::decode(compressed);
    EXPECT_EQ(decoded, doc_ids);
}

TEST_F(PForDeltaTest, DecodeInvalidData) {
    // Invalid p value (0)
    std::vector<uint8_t> invalid_compressed = {0, 255, 255};
    EXPECT_THROW(PForDelta::decode(invalid_compressed), std::invalid_argument);
}

TEST_F(PForDeltaTest, DecodeInsufficientBits) {
    // p=4, but only provides 2 bits for a value requiring 4 bits
    std::vector<uint8_t> invalid_compressed = {4, 0x0F}; // p=4, one byte with value 15 (00001111)
    EXPECT_THROW(PForDelta::decode(invalid_compressed), std::invalid_argument);
}

} // namespace inverted_index
