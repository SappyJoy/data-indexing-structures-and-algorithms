// File: test/Testing/InvertedIndexTest.cpp

#include "inverted_index/InvertedIndex.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>

namespace inverted_index {

class InvertedIndexTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Initialize logger for testing
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
    }

    InvertedIndex index;
};

TEST_F(InvertedIndexTest, AddSingleDocument) {
    index.addDocument(1, "Hello, World!");

    EXPECT_TRUE(index.contains("hello"));
    EXPECT_TRUE(index.contains("world"));
    EXPECT_FALSE(index.contains("nonexistent"));

    const auto &hello_postings = index.getPostings("hello");
    ASSERT_EQ(hello_postings.size(), 1);
    EXPECT_EQ(hello_postings[0], 1);

    const auto &world_postings = index.getPostings("world");
    ASSERT_EQ(world_postings.size(), 1);
    EXPECT_EQ(world_postings[0], 1);
}

TEST_F(InvertedIndexTest, AddMultipleDocuments) {
    index.addDocument(1, "Hello, World!");
    index.addDocument(2, "Hello again!");
    index.addDocument(3, "Goodbye, World!");

    EXPECT_TRUE(index.contains("hello"));
    EXPECT_TRUE(index.contains("world"));
    EXPECT_TRUE(index.contains("again"));
    EXPECT_TRUE(index.contains("goodbye"));
    EXPECT_FALSE(index.contains("missing"));

    const auto &hello_postings = index.getPostings("hello");
    ASSERT_EQ(hello_postings.size(), 2);
    EXPECT_EQ(hello_postings[0], 1);
    EXPECT_EQ(hello_postings[1], 2);

    const auto &world_postings = index.getPostings("world");
    ASSERT_EQ(world_postings.size(), 2);
    EXPECT_EQ(world_postings[0], 1);
    EXPECT_EQ(world_postings[1], 3);

    const auto &goodbye_postings = index.getPostings("goodbye");
    ASSERT_EQ(goodbye_postings.size(), 1);
    EXPECT_EQ(goodbye_postings[0], 3);
}

TEST_F(InvertedIndexTest, HandleDuplicateDocIDs) {
    index.addDocument(1, "Hello, World!");
    index.addDocument(1, "Hello again!");

    EXPECT_TRUE(index.contains("hello"));
    EXPECT_TRUE(index.contains("world"));
    EXPECT_TRUE(index.contains("again"));

    const auto &hello_postings = index.getPostings("hello");
    ASSERT_EQ(hello_postings.size(), 1);
    EXPECT_EQ(hello_postings[0], 1);

    const auto &world_postings = index.getPostings("world");
    ASSERT_EQ(world_postings.size(), 1);
    EXPECT_EQ(world_postings[0], 1);

    const auto &again_postings = index.getPostings("again");
    ASSERT_EQ(again_postings.size(), 1);
    EXPECT_EQ(again_postings[0], 1);
}

TEST_F(InvertedIndexTest, EmptyText) {
    index.addDocument(1, "");

    // No terms should be added
    EXPECT_FALSE(index.contains("anyterm"));
}

TEST_F(InvertedIndexTest, OnlyPunctuation) {
    index.addDocument(1, "!!!,,,???...");

    // No terms should be added
    EXPECT_FALSE(index.contains("anyterm"));
}

TEST_F(InvertedIndexTest, Tokenization) {
    index.addDocument(1, "Don't tell your mother to fuck off, say thank you for the cutlets!");

    EXPECT_TRUE(index.contains("mother"));
    EXPECT_TRUE(index.contains("cutlets"));
    EXPECT_TRUE(index.contains("thank"));
    EXPECT_FALSE(index.contains("programming"));

    const auto &mother_postings = index.getPostings("mother");
    ASSERT_EQ(mother_postings.size(), 1);
    EXPECT_EQ(mother_postings[0], 1);

    const auto &cutlets_postings = index.getPostings("cutlets");
    ASSERT_EQ(cutlets_postings.size(), 1);
    EXPECT_EQ(cutlets_postings[0], 1);
}

} // namespace inverted_index
