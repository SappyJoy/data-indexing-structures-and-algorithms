#include "inverted_index/QueryProcessor.hpp"
#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/StorageManager.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace inverted_index {

// Helper function to create a sample InvertedIndex with predefined documents
void createSampleIndex(InvertedIndex &index) {
    index.addDocument(1, "the quick brown fox");
    index.addDocument(2, "jumps over the lazy dog");
    index.addDocument(3, "hello world");
    index.addDocument(4, "hello again");
    index.addDocument(5, "fox and dog");
}

// Helper function to compare two vectors irrespective of order
bool compareVectors(const std::vector<int> &v1, const std::vector<int> &v2) {
    if (v1.size() != v2.size())
        return false;
    std::vector<int> sorted_v1 = v1;
    std::vector<int> sorted_v2 = v2;
    std::sort(sorted_v1.begin(), sorted_v1.end());
    std::sort(sorted_v2.begin(), sorted_v2.end());
    return sorted_v1 == sorted_v2;
}

// Test Fixture for QueryProcessor Tests
class QueryProcessorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        try {
            Logger::getInstance().enableFileLogging("logs/test_query_processor.log");
        } catch (const std::exception &e) {
            std::cerr << "Logger initialization failed: " << e.what() << std::endl;
            // Handle logger initialization failure if necessary
        }
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);

        // Create and populate the InvertedIndex
        inverted_index::createSampleIndex(index);

        // Initialize the QueryProcessor with the populated index
        query_processor = std::make_unique<inverted_index::QueryProcessor>(index);
    }

    void TearDown() override {}

    inverted_index::InvertedIndex index;
    std::unique_ptr<inverted_index::QueryProcessor> query_processor;
};

// Test Case 1: Single-Term Queries
TEST_F(QueryProcessorTest, SingleTermQueries) {
    // Query for "hello"
    std::string query1 = "hello";
    std::vector<int> expected1 = {3, 4};
    auto result1 = query_processor->executeQuery(query1);
    EXPECT_TRUE(inverted_index::compareVectors(result1, expected1))
        << "Query: \"" << query1 << "\" Expected: {3, 4} Got: {"
        << ((result1.empty()) ? "" : std::to_string(result1[0])) << "}";

    // Query for "fox"
    std::string query2 = "fox";
    std::vector<int> expected2 = {1, 5};
    auto result2 = query_processor->executeQuery(query2);
    EXPECT_TRUE(inverted_index::compareVectors(result2, expected2))
        << "Query: \"" << query2 << "\" Expected: {1, 5} Got: {"
        << ((result2.empty()) ? "" : std::to_string(result2[0])) << "}";
}

// Test Case 2: AND Operator
TEST_F(QueryProcessorTest, AndOperator) {
    // Query: "hello AND dog"
    std::string query = "hello AND dog";
    std::vector<int> expected = {}; // No document contains both "hello" and "dog"
    auto result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected)) << "Query: \"" << query << "\" Expected: {} Got: {";

    // Adding a document that contains both "hello" and "dog"
    index.addDocument(6, "hello dog");

    // Query again
    expected = {6};
    result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected))
        << "After adding document 6, Query: \"" << query << "\" Expected: {6} Got: {";
}

// Test Case 3: OR Operator
TEST_F(QueryProcessorTest, OrOperator) {
    // Query: "quick OR lazy"
    std::string query = "quick OR lazy";
    std::vector<int> expected = {1, 2};
    auto result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected))
        << "Query: \"" << query << "\" Expected: {1, 2} Got: {";

    // Query: "hello OR fox"
    std::string query2 = "hello OR fox";
    std::vector<int> expected2 = {1, 3, 4, 5};
    auto result2 = query_processor->executeQuery(query2);
    EXPECT_TRUE(inverted_index::compareVectors(result2, expected2))
        << "Query: \"" << query2 << "\" Expected: {1, 3, 4, 5} Got: {";
}

// Test Case 4: NOT Operator
TEST_F(QueryProcessorTest, NotOperator) {
    // Query: "NOT fox"
    std::string query = "NOT fox";
    // Assuming total documents = 5
    // Documents not containing "fox" are 2, 3, 4
    std::vector<int> expected = {2, 3, 4};
    auto result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected))
        << "Query: \"" << query << "\" Expected: {2, 3, 4} Got: {";

    // Query: "hello AND NOT dog"
    std::string query2 = "hello AND NOT dog";
    // Documents containing "hello" are 3,4
    // Documents containing "dog" are 2,5
    // Documents containing "hello" and not "dog" are 3,4
    std::vector<int> expected2 = {3, 4};
    auto result2 = query_processor->executeQuery(query2);
    EXPECT_TRUE(inverted_index::compareVectors(result2, expected2))
        << "Query: \"" << query2 << "\" Expected: {3, 4} Got: {";
}

// Test Case 5: Combined Operators and Parentheses
TEST_F(QueryProcessorTest, CombinedOperatorsAndParentheses) {
    // Query: "(hello OR fox) AND dog"
    std::string query = "(hello OR fox) AND dog";
    // Documents containing "hello" OR "fox" are {1,3,4,5}
    // Documents containing "dog" are {2,5}
    // Intersection is {5}
    std::vector<int> expected = {5};
    auto result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected)) << "Query: \"" << query << "\" Expected: {5} Got: {";

    // Query: "hello AND (world OR again)"
    std::string query2 = "hello AND (world OR again)";
    // Documents containing "hello" are {3,4}
    // Documents containing "world" are {3}
    // Documents containing "again" are {4}
    // Union of "world" OR "again" is {3,4}
    // Intersection with "hello" is {3,4}
    std::vector<int> expected2 = {3, 4};
    auto result2 = query_processor->executeQuery(query2);
    EXPECT_TRUE(inverted_index::compareVectors(result2, expected2))
        << "Query: \"" << query2 << "\" Expected: {3, 4} Got: {";
}

// Test Case 6: Queries with Non-Existent Terms
TEST_F(QueryProcessorTest, NonExistentTerms) {
    // Query: "cat AND dog"
    std::string query = "cat AND dog";
    std::vector<int> expected = {}; // "cat" does not exist
    auto result = query_processor->executeQuery(query);
    EXPECT_TRUE(inverted_index::compareVectors(result, expected)) << "Query: \"" << query << "\" Expected: {} Got: {";

    // Query: "hello AND unicorn"
    std::string query2 = "hello AND unicorn";
    std::vector<int> expected2 = {}; // "unicorn" does not exist
    auto result2 = query_processor->executeQuery(query2);
    EXPECT_TRUE(inverted_index::compareVectors(result2, expected2))
        << "Query: \"" << query2 << "\" Expected: {} Got: {";
}

// Test Case 7: Malformed Queries
TEST_F(QueryProcessorTest, MalformedQueries) {
    // Query: "AND dog"
    std::string query1 = "AND dog";
    EXPECT_THROW({ query_processor->executeQuery(query1); }, std::invalid_argument);

    // Query: "hello OR"
    std::string query2 = "hello OR";
    EXPECT_THROW({ query_processor->executeQuery(query2); }, std::invalid_argument);

    // Query: "hello AND (dog OR"
    std::string query3 = "hello AND (dog OR";
    EXPECT_THROW({ query_processor->executeQuery(query3); }, std::invalid_argument);

    // Query: "hello OR (dog AND )"
    std::string query4 = "hello OR (dog AND )";
    EXPECT_THROW({ query_processor->executeQuery(query4); }, std::invalid_argument);

    // Query: "hello NOT AND dog"
    std::string query5 = "hello NOT AND dog";
    EXPECT_THROW({ query_processor->executeQuery(query5); }, std::invalid_argument);
}

} // namespace inverted_index
