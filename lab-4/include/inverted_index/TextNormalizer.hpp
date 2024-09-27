#ifndef INVERTED_INDEX_TEXTNORMALIZER_HPP
#define INVERTED_INDEX_TEXTNORMALIZER_HPP

#include <string>

namespace inverted_index {

/**
 * @brief A utility class for normalizing text.
 *
 * This class provides functionality to convert text to lowercase
 * and remove punctuation characters.
 */
class TextNormalizer {
  public:
    /**
     * @brief Normalizes the input text by converting it to lowercase
     *        and removing punctuation.
     *
     * @param text The input string to normalize.
     * @return A normalized string with lowercase letters and no punctuation.
     */
    static std::string normalize(const std::string &text);
};

} // namespace inverted_index

#endif // INVERTED_INDEX_TEXTNORMALIZER_HPP
