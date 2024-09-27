#ifndef INVERTED_INDEX_SKIPLISTS_HPP
#define INVERTED_INDEX_SKIPLISTS_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace inverted_index {

/**
 * @brief Represents a skip pointer with a document ID and its byte offset in the compressed posting list.
 */
struct SkipPointer {
    int doc_id;         ///< The document ID where the skip pointer points.
    size_t byte_offset; ///< The byte offset in the compressed posting list where the doc_id is located.
};

/**
 * @brief A class that manages skip pointers for posting lists to enhance query performance.
 */
class Skiplists {
  public:
    /**
     * @brief Adds a skip pointer for a given term.
     *
     * @param term The term for which the skip pointer is added.
     * @param skip_pointer The skip pointer to add.
     */
    void addSkipPointer(const std::string &term, const SkipPointer &skip_pointer);

    /**
     * @brief Retrieves the skip pointers for a given term.
     *
     * @param term The term for which to retrieve skip pointers.
     * @return A constant reference to the vector of skip pointers for the term.
     *         Returns an empty vector if no skip pointers exist for the term.
     */
    const std::vector<SkipPointer> &getSkipPointers(const std::string &term) const;

    /**
     * @brief Checks if a term has skip pointers.
     *
     * @param term The term to check.
     * @return True if the term has skip pointers, false otherwise.
     */
    bool hasSkipPointers(const std::string &term) const;

    /**
     * @brief Clears all skip pointers for all terms.
     */
    void clear();

    /**
     * @brief Builds skip pointers for a term based on its compressed posting list.
     *
     * This function parses the compressed_data and identifies skip points based on the block structure.
     * Each block's starting document ID and byte offset are added as skip pointers.
     *
     * @param term The term for which to build skip pointers.
     * @param compressed_data The compressed posting list data.
     */
    void buildSkipPointers(const std::string &term, const std::vector<uint8_t> &compressed_data);

    /**
     * @brief Adds multiple skip pointers for a given term.
     *
     * @param term The term for which to add skip pointers.
     * @param skips The vector of SkipPointer structs.
     */
    void addSkipPointers(const std::string &term, const std::vector<SkipPointer> &skips);

  private:
    // Mapping from term to its skip pointers
    std::unordered_map<std::string, std::vector<SkipPointer>> skip_map_;
};

} // namespace inverted_index

#endif
