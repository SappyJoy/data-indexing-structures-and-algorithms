#include "inverted_index/InvertedIndex.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>

namespace inverted_index {

class InvertedIndexIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for testing
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
    }

    InvertedIndex index;
};

TEST_F(InvertedIndexIntegrationTest, AddAndRetrieveDocuments) {
    // Add multiple documents
    index.addDocument(1, "Hello, World!");
    index.addDocument(2, "The quick brown fox jumps over the lazy dog.");
    index.addDocument(3, "C++ is a powerful programming language.");
    index.addDocument(4, "Hello again! This document is the second one.");
    index.addDocument(5, "Testing the inverted index implementation.");

    // Test retrieval
    std::vector<int> hello_postings = index.getPostings("hello");
    EXPECT_EQ(hello_postings, std::vector<int>({1, 4}));

    std::vector<int> world_postings = index.getPostings("world");
    EXPECT_EQ(world_postings, std::vector<int>({1}));

    std::vector<int> the_postings = index.getPostings("the");
    EXPECT_EQ(the_postings, std::vector<int>({2, 4, 5}));

    std::vector<int> missing_postings = index.getPostings("missing");
    EXPECT_TRUE(missing_postings.empty());
}

TEST_F(InvertedIndexIntegrationTest, HandleDuplicateDocIDs) {
    // Add the same document multiple times
    index.addDocument(1, "Hello, World!");
    index.addDocument(1, "Hello, World!"); // Duplicate

    // Test that the posting lists do not have duplicates
    std::vector<int> hello_postings = index.getPostings("hello");
    EXPECT_EQ(hello_postings, std::vector<int>({1}));

    std::vector<int> world_postings = index.getPostings("world");
    EXPECT_EQ(world_postings, std::vector<int>({1}));
}

TEST_F(InvertedIndexIntegrationTest, HandleEmptyAndPunctuationOnlyDocuments) {
    // Add empty document
    index.addDocument(6, "");

    // Add punctuation-only document
    index.addDocument(7, "!!!,,,???...");

    // Ensure no terms are added
    EXPECT_FALSE(index.contains("anyterm"));
}

TEST_F(InvertedIndexIntegrationTest, LargePostingList) {
    // Add multiple documents containing the same term
    for (int doc_id = 1; doc_id <= 1000; ++doc_id) {
        index.addDocument(doc_id, "commonterm");
    }

    // Retrieve posting list
    std::vector<int> common_postings = index.getPostings("commonterm");
    EXPECT_EQ(common_postings.size(), 1000);
    for (int doc_id = 1; doc_id <= 1000; ++doc_id) {
        EXPECT_EQ(common_postings[doc_id - 1], doc_id);
    }
}

} // namespace inverted_index

