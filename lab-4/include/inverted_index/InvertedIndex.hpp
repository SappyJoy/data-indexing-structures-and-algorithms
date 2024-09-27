#ifndef INVERTED_INDEX_INVERTEDINDEX_HPP
#define INVERTED_INDEX_INVERTEDINDEX_HPP

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace inverted_index {

/**
 * @brief Represents an inverted index mapping terms to their compressed posting lists.
 */
class InvertedIndex {
  public:
    /**
     * @brief Default constructor for InvertedIndex.
     */
    InvertedIndex();

    /**
     * @brief Adds a document to the inverted index.
     *
     * @param doc_id The unique identifier for the document.
     * @param text The textual content of the document.
     */
    void addDocument(int doc_id, const std::string &text);

    /**
     * @brief Retrieves the posting list for a given term.
     *
     * @param term The term for which to retrieve the posting list.
     * @return A constant reference to the vector of document IDs containing the term.
     *         Returns an empty vector if the term does not exist.
     */
    std::vector<int> getPostings(const std::string &term) const;

    /**
     * @brief Checks if a term exists in the inverted index.
     *
     * @param term The term to check.
     * @return True if the term exists, false otherwise.
     */
    bool contains(const std::string &term) const;

  private:
    // Mapping from term to its compressed posting list (vector of bytes)
    std::unordered_map<std::string, std::vector<uint8_t>> index_;

    // Mutex for thread-safe access to the index
    mutable std::shared_mutex mutex_;

    /**
     * @brief Tokenizes a normalized text into individual terms based on whitespace.
     *
     * @param text The normalized text to tokenize.
     * @return A vector of terms extracted from the text.
     */
    std::vector<std::string> tokenize(const std::string &text) const;
};

} // namespace inverted_index

#endif
