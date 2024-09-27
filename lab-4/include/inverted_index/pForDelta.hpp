#ifndef INVERTED_INDEX_PFORDELTA_HPP
#define INVERTED_INDEX_PFORDELTA_HPP

#include <cstdint>
#include <vector>

namespace inverted_index {

/**
 * @brief A class that implements pForDelta encoding and decoding for compression of posting lists.
 */
class PForDelta {
  public:
    /**
     * @brief Encodes a list of sorted document IDs into a compressed byte stream using pForDelta.
     *
     * @param doc_ids A sorted vector of document IDs.
     * @return A vector of bytes representing the compressed posting list.
     */
    static std::vector<uint8_t> encode(const std::vector<int> &doc_ids);

    /**
     * @brief Decodes a compressed byte stream back into a list of document IDs using pForDelta.
     *
     * @param compressed_data The compressed byte stream.
     * @return A sorted vector of document IDs.
     */
    static std::vector<int> decode(const std::vector<uint8_t> &compressed_data);
};

} // namespace inverted_index

#endif
