#include "inverted_index/TextNormalizer.hpp"
#include <gtest/gtest.h>

namespace inverted_index {

TEST(TextNormalizerTest, EmptyString) {
    std::string input = "";
    std::string expected = "";
    EXPECT_EQ(TextNormalizer::normalize(input), expected);
}

TEST(TextNormalizerTest, OnlyPunctuation) {
    std::string input = "!!!,,,???...";
    std::string expected = "";
    EXPECT_EQ(TextNormalizer::normalize(input), expected);
}

TEST(TextNormalizerTest, MixedContent) {
    std::string input = "Hello, World! This is a Test.";
    std::string expected = "hello world this is a test";
    EXPECT_EQ(TextNormalizer::normalize(input), expected);
}

TEST(TextNormalizerTest, AlreadyNormalized) {
    std::string input = "hello world";
    std::string expected = "hello world";
    EXPECT_EQ(TextNormalizer::normalize(input), expected);
}

TEST(TextNormalizerTest, MultipleSpaces) {
    std::string input = "Hello    World!!!  This   is   a Test.";
    std::string expected = "hello world this is a test";
    EXPECT_EQ(TextNormalizer::normalize(input), expected);
}

} // namespace inverted_index
