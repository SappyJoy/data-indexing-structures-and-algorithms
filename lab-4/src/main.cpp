#include "inverted_index/InvertedIndex.hpp"
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

    // Sample documents
    std::vector<std::pair<int, std::string>> documents = {
        {1, "Hello, World! This is the first document."},
        {2, "The quick brown fox jumps over the lazy dog."},
        {3, "C++ is a powerful programming language."},
        {4, "Hello again! This document is the second one."},
        {5, "Testing the inverted index implementation."}
    };

    // Add documents to the index
    for (const auto& [doc_id, text] : documents) {
        index.addDocument(doc_id, text);
    }

    // Query some terms
    std::vector<std::string> query_terms = {"hello", "document", "cpp", "quick", "missing"};

    for (const auto& term : query_terms) {
        std::vector<int> postings = index.getPostings(term);
        if (!postings.empty()) {
            LOG_INFO("Term '{}' found in documents: ", term);
            for (const auto& doc_id : postings) {
                std::cout << "  - Document ID: " << doc_id << "\n";
            }
        }
        else {
            LOG_INFO("Term '{}' not found in any document.", term);
        }
    }

    LOG_INFO("Application terminated");

    return 0;
}
