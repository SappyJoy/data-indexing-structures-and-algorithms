#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/TextNormalizer.hpp"
#include "inverted_index/pForDelta.hpp"
#include "log/Logger.hpp"

#include <algorithm>
#include <sstream>

namespace inverted_index {

InvertedIndex::InvertedIndex() { LOG_DEBUG("InvertedIndex initialized with pForDelta compression."); }

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
        std::vector<int> posting_list = PForDelta::decode(compressed_posting);

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
        compressed_posting = PForDelta::encode(posting_list);
    }

    LOG_INFO("Document ID {} added successfully.", doc_id);
}

std::vector<int> InvertedIndex::getPostings(const std::string &term) const {
    // Acquire shared lock for reading
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto it = index_.find(term);
    if (it != index_.end()) {
        LOG_DEBUG("Retrieved postings for term '{}'.", term);
        return PForDelta::decode(it->second);
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

} // namespace inverted_index
