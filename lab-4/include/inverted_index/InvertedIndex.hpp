#ifndef INVERTED_INDEX_INVERTEDINDEX_HPP
#define INVERTED_INDEX_INVERTEDINDEX_HPP

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Skiplists.hpp"

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
     * @return A sorted vector of document IDs containing the term.
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

    /**
     * @brief Retrieves the internal index map.
     *
     * @return A constant reference to the index map.
     */
    const std::unordered_map<std::string, std::vector<uint8_t>>& getIndexMap() const;

    /**
     * @brief Retrieves the internal Skiplists instance.
     *
     * @return A constant reference to the Skiplists.
     */
    const Skiplists& getSkiplists() const;

    /**
     * @brief Inserts a term and its compressed posting list into the index.
     *
     * @param term The term to insert.
     * @param compressed_posting The compressed posting list for the term.
     */
    void insertTerm(const std::string& term, const std::vector<uint8_t>& compressed_posting);

    /**
     * @brief Inserts skip pointers for a given term.
     *
     * @param term The term for which to insert skip pointers.
     * @param skips The vector of SkipPointer structs.
     */
    void insertSkips(const std::string& term, const std::vector<SkipPointer>& skips);


  private:
    // Mapping from term to its compressed posting list (vector of bytes)
    std::unordered_map<std::string, std::vector<uint8_t>> index_;

    // Skiplists instance to manage skip pointers
    Skiplists skiplists_;

    // Mutex for thread-safe access to the index and skiplists
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
