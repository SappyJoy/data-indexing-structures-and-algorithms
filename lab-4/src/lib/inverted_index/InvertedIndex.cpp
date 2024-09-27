#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/TextNormalizer.hpp"
#include "inverted_index/pForDelta.hpp"
#include "log/Logger.hpp"

#include <algorithm>
#include <sstream>

namespace inverted_index {

InvertedIndex::InvertedIndex() : total_documents_(0) {
    LOG_DEBUG("InvertedIndex initialized with pForDelta compression and Skiplists.");
}

void InvertedIndex::addDocument(int doc_id, const std::string &text) {
    LOG_DEBUG("Adding document ID {} to the inverted index.", doc_id);

    // Normalize the text
    std::string normalized_text = TextNormalizer::normalize(text);
    LOG_DEBUG("Normalized text: '{}'", normalized_text);

    if (normalized_text.empty()) {
        LOG_WARNING("Document ID {} has no valid terms after normalization.", doc_id);
        return;
    }

    // Tokenize the normalized text into terms
    std::vector<std::string> terms = tokenize(normalized_text);
    LOG_DEBUG("Document ID {} contains {} terms after tokenization.", doc_id, terms.size());

    for (const auto &term : terms) {
        if (term.empty()) {
            continue; // Skip empty terms
        }

        // Acquire exclusive lock for writing
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto &compressed_posting = index_[term];

        // Decompress current posting list
        std::vector<int> posting_list;
        if (!compressed_posting.empty()) {
            try {
                posting_list = PForDelta::decode(compressed_posting);
            } catch (const std::invalid_argument& e) {
                LOG_ERROR("Failed to decode posting list for term '{}': {}", term, e.what());
                continue;
            }
        }

        // Insert doc_id in sorted order without duplicates
        if (posting_list.empty() || posting_list.back() < doc_id) {
            posting_list.push_back(doc_id);
            LOG_DEBUG("Appended doc ID {} to term '{}'.", doc_id, term);
        } else if (posting_list.back() == doc_id) {
            LOG_DEBUG("Document ID {} already exists in term '{}'. Skipping duplicate.", doc_id, term);
            continue;
        } else {
            // Insert in sorted order
            auto it = std::lower_bound(posting_list.begin(), posting_list.end(), doc_id);
            if (it == posting_list.end() || *it != doc_id) {
                posting_list.insert(it, doc_id);
                LOG_DEBUG("Inserted doc ID {} into term '{}' at position {}.", doc_id, term,
                          std::distance(posting_list.begin(), it));
            } else {
                LOG_DEBUG("Document ID {} already exists in term '{}'. Skipping duplicate.", doc_id, term);
            }
        }

        // Recompress the posting list
        std::vector<uint8_t> new_compressed_posting;
        try {
            new_compressed_posting = PForDelta::encode(posting_list);
        } catch (const std::invalid_argument& e) {
            LOG_ERROR("Failed to encode posting list for term '{}': {}", term, e.what());
            continue;
        }

        index_[term] = std::move(new_compressed_posting);

        // Build skip pointers for the term based on the new compressed data
        try {
            skiplists_.buildSkipPointers(term, index_[term]);
        } catch (const std::invalid_argument& e) {
            LOG_ERROR("Failed to build Skiplists for term '{}': {}", term, e.what());
            // Optionally, handle the error or continue
            continue;
        }
    }
    ++total_documents_;

    LOG_INFO("Document ID {} added successfully.", doc_id);
}

std::vector<int> InvertedIndex::getPostings(const std::string &term) const {
    // Acquire shared lock for reading
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto it = index_.find(term);
    if (it != index_.end()) {
        LOG_DEBUG("Retrieved postings for term '{}'.", term);
        try {
            return PForDelta::decode(it->second);
        } catch (const std::invalid_argument& e) {
            LOG_ERROR("Failed to decode posting list for term '{}': {}", term, e.what());
            return {};
        }
    }

    LOG_DEBUG("Term '{}' not found in the inverted index.", term);
    return {};
}

bool InvertedIndex::contains(const std::string &term) const {
    // Acquire shared lock for reading
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return index_.find(term) != index_.end();
}

std::vector<std::string> InvertedIndex::tokenize(const std::string &text) const {
    std::vector<std::string> tokens;
    std::istringstream stream(text);
    std::string token;

    while (stream >> token) {
        tokens.emplace_back(token);
    }

    return tokens;
}


const std::unordered_map<std::string, std::vector<uint8_t>>& InvertedIndex::getIndexMap() const {
    return index_;
}

const Skiplists& InvertedIndex::getSkiplists() const {
    return skiplists_;
}

void InvertedIndex::insertTerm(const std::string& term, const std::vector<uint8_t>& compressed_posting) {
    std::unique_lock lock(mutex_);
    index_[term] = compressed_posting;
}

void InvertedIndex::insertSkips(const std::string& term, const std::vector<SkipPointer>& skips) {
    std::unique_lock lock(mutex_);
    skiplists_.addSkipPointers(term, skips); // Implement a method to add multiple skips
}

int InvertedIndex::getTotalDocuments() const {
    std::shared_lock lock(mutex_);
    return total_documents_;
}

} // namespace inverted_index
