#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/StorageManager.hpp"
#include "inverted_index/Skiplists.hpp"
#include "log/Logger.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace inverted_index {

// Helper function to create a sample InvertedIndex with predefined documents
void createSampleIndex(InvertedIndex& index) {
    index.addDocument(1, "Hello, World! This is the first document.");
    index.addDocument(2, "The quick brown fox jumps over the lazy dog.");
    index.addDocument(3, "C++ is a powerful programming language.");
    index.addDocument(4, "Hello again! This document is the second one.");
    index.addDocument(5, "Testing the inverted index implementation.");
}

// Helper function to compare two InvertedIndex instances
bool compareIndices(const InvertedIndex& index1, const InvertedIndex& index2) {
    const auto& map1 = index1.getIndexMap();
    const auto& map2 = index2.getIndexMap();
    
    if (map1.size() != map2.size()) {
        return false;
    }

    for (const auto& [term, posting1] : map1) {
        auto it = map2.find(term);
        if (it == map2.end()) {
            return false;
        }
        const auto& posting2 = it->second;
        if (posting1 != posting2) {
            return false;
        }
    }

    // Compare Skiplists
    const Skiplists& skips1 = index1.getSkiplists();
    const Skiplists& skips2 = index2.getSkiplists();

    for (const auto& [term, posting1] : map1) {
        const auto& skips_term1 = skips1.getSkipPointers(term);
        const auto& skips_term2 = skips2.getSkipPointers(term);
        if (skips_term1.size() != skips_term2.size()) {
            return false;
        }
        for (size_t i = 0; i < skips_term1.size(); ++i) {
            if (skips_term1[i].doc_id != skips_term2[i].doc_id ||
                skips_term1[i].byte_offset != skips_term2[i].byte_offset) {
                return false;
            }
        }
    }

    return true;
}


// Test Fixture for StorageManager Tests
class StorageManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            Logger::getInstance().enableFileLogging("logs/test_storage_manager.log");
        } catch (const std::exception& e) {
            std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        }
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
    }

    void TearDown() override {
        // Clean up log files or other resources if necessary
        // Remove the temporary file after each test
        if (std::filesystem::exists(temp_file)) {
            std::filesystem::remove(temp_file);
        }
    }

    // Temporary file path for testing
    const std::string temp_file = "test_index.dat";
};

// Test Case 1: Saving and Loading an Empty Index
TEST_F(StorageManagerTest, SaveAndLoadEmptyIndex) {
    inverted_index::InvertedIndex empty_index;

    // Save the empty index
    EXPECT_NO_THROW({
        inverted_index::StorageManager::saveIndex(empty_index, temp_file);
    });

    // Load into a new index instance
    inverted_index::InvertedIndex loaded_index;
    EXPECT_NO_THROW({
        inverted_index::StorageManager::loadIndex(loaded_index, temp_file);
    });

    // Compare the empty indices
    EXPECT_TRUE(inverted_index::compareIndices(empty_index, loaded_index));
}

// Test Case 2: Saving and Loading a Non-Empty Index
TEST_F(StorageManagerTest, SaveAndLoadNonEmptyIndex) {
    inverted_index::InvertedIndex original_index;
    inverted_index::createSampleIndex(original_index);

    // Save the original index
    EXPECT_NO_THROW({
        inverted_index::StorageManager::saveIndex(original_index, temp_file);
    });

    // Load into a new index instance
    inverted_index::InvertedIndex loaded_index;
    EXPECT_NO_THROW({
        inverted_index::StorageManager::loadIndex(loaded_index, temp_file);
    });

    // Compare the original and loaded indices
    EXPECT_TRUE(inverted_index::compareIndices(original_index, loaded_index));
}

// Test Case 3: Loading from a Non-Existent File
TEST_F(StorageManagerTest, LoadFromNonExistentFile) {
    inverted_index::InvertedIndex index;
    std::string nonexistent_file = "nonexistent_index.dat";

    // Ensure the file does not exist
    if (std::filesystem::exists(nonexistent_file)) {
        std::filesystem::remove(nonexistent_file);
    }

    // Attempt to load from a non-existent file, expecting an exception
    EXPECT_THROW({
        inverted_index::StorageManager::loadIndex(index, nonexistent_file);
    }, std::runtime_error);
}

// Test Case 4: Loading from a Corrupted File
TEST_F(StorageManagerTest, LoadFromCorruptedFile) {
    // Create a corrupted file with invalid magic number
    std::ofstream out(temp_file, std::ios::binary);
    ASSERT_TRUE(out.is_open());
    out.write("BAD1", 4); // Invalid magic number
    out.close();

    inverted_index::InvertedIndex index;

    // Attempt to load from the corrupted file, expecting an exception
    EXPECT_THROW({
        inverted_index::StorageManager::loadIndex(index, temp_file);
    }, std::runtime_error);
}

// Test Case 5: Ensuring Data Integrity After Save and Load
TEST_F(StorageManagerTest, DataIntegrityAfterSaveAndLoad) {
    inverted_index::InvertedIndex original_index;
    inverted_index::createSampleIndex(original_index);

    // Save the original index
    EXPECT_NO_THROW({
        inverted_index::StorageManager::saveIndex(original_index, temp_file);
    });

    // Load into a new index instance
    inverted_index::InvertedIndex loaded_index;
    EXPECT_NO_THROW({
        inverted_index::StorageManager::loadIndex(loaded_index, temp_file);
    });

    // Ensure that the loaded index matches the original
    EXPECT_TRUE(inverted_index::compareIndices(original_index, loaded_index));
}

} // namespace inverted_index

