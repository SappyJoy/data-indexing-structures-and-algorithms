#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/StorageManager.hpp"
#include "inverted_index/QueryProcessor.hpp"
#include "log/Logger.hpp"
#include <iostream>

int main() {
    using namespace inverted_index;

    // Enable file logging (optional)
    try {
        Logger::getInstance().enableFileLogging("logs/app.log");
    } catch (const std::exception &e) {
        std::cerr << "Logging setup failed: " << e.what() << std::endl;
        return 1;
    }

    // Set log level to DEBUG to capture all log messages
    Logger::getInstance().setLogLevel(LogLevel::DEBUG);

    LOG_INFO("Application started");

    InvertedIndex index;

    // Add sample documents
    std::vector<std::pair<int, std::string>> documents = {
        {1, "Hello, World! This is the first document."},
        {2, "The quick brown fox jumps over the lazy dog."},
        {3, "C++ is a powerful programming language."},
        {4, "Hello again! This document is the second one."},
        {5, "Testing the inverted index implementation."}
    };

    for (const auto& [doc_id, text] : documents) {
        index.addDocument(doc_id, text);
    }

    // Save the index to disk
    try {
        StorageManager::saveIndex(index, "index.dat");
        LOG_INFO("Index successfully saved to 'index.dat'.");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save index: {}", e.what());
        return 1;
    }

    // Create a new InvertedIndex instance to load data
    InvertedIndex loaded_index;

    // Load the index from disk
    try {
        StorageManager::loadIndex(loaded_index, "index.dat");
        LOG_INFO("Index successfully loaded from 'index.dat'.");
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load index: {}", e.what());
        return 1;
    }

    // Query the loaded index
    std::vector<std::string> query_terms = {"hello", "document", "cpp", "quick", "missing"};
    for (const auto& term : query_terms) {
        std::vector<int> postings = loaded_index.getPostings(term);
        if (!postings.empty()) {
            LOG_INFO("Term '{}' found in documents:", term);
            for (const auto& doc_id : postings) {
                std::cout << "  - Document ID: " << doc_id << "\n";
            }
        }
        else {
            LOG_INFO("Term '{}' not found in any document.", term);
        }
    }

    // Initialize QueryProcessor
    QueryProcessor query_processor(loaded_index);

    // Sample queries
    std::vector<std::string> queries = {
        "hello AND document",
        "quick OR lazy",
        "C++ AND NOT fox",
        "programming OR (hello AND NOT world)"
    };

    for (const auto& query : queries) {
        try {
            auto results = query_processor.executeQuery(query);
            std::cout << "Query: \"" << query << "\" found in documents: ";
            for (const auto& doc_id : results) {
                std::cout << doc_id << " ";
            }
            std::cout << "\n";
            LOG_INFO("Executed query: '{}', found in {} documents.", query, results.size());
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to execute query '{}': {}", query, e.what());
        }
    }

    LOG_INFO("Application terminated.");
    return 0;
}
