#include "inverted_index/TextNormalizer.hpp"
#include "log/Logger.hpp"

#include <cctype>
#include <string>

namespace inverted_index {

    std::string TextNormalizer::normalize(const std::string& text) {
        LOG_DEBUG("Starting text normalization.");
        if (text.empty()) {
            LOG_WARNING("Received an empty string for normalization.");
            return "";
        }

        std::string normalized;
        normalized.reserve(text.size()); // Reserve space to improve performance

        for (size_t i = 0; i < text.size(); ++i) {
            char ch = text[i];
            if (std::isspace(static_cast<unsigned char>(ch))) {
                // Optionally, you can decide to keep spaces or replace them with a single space
                normalized += ' ';
                continue;
            }

            if (std::ispunct(static_cast<unsigned char>(ch))) {
                LOG_DEBUG("Removing punctuation character: '{}'", ch);
                continue; // Skip punctuation
            }

            // Convert to lowercase
            char lower_ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            normalized += lower_ch;
        }

        // Remove any extra spaces that may have been added
        // For simplicity, let's trim leading and trailing spaces and replace multiple spaces with single space
        size_t start = normalized.find_first_not_of(' ');
        size_t end = normalized.find_last_not_of(' ');
        if (start == std::string::npos) {
            LOG_WARNING("String contains only punctuation or whitespace after normalization.");
            return "";
        }

        normalized = normalized.substr(start, end - start + 1);

        // Replace multiple spaces with a single space
        std::string final_normalized;
        final_normalized.reserve(normalized.size());

        bool in_space = false;
        for (char ch : normalized) {
            if (ch == ' ') {
                if (!in_space) {
                    final_normalized += ch;
                    in_space = true;
                }
                // Else skip additional spaces
            } else {
                final_normalized += ch;
                in_space = false;
            }
        }

        LOG_INFO("Text normalization completed. Original size: {}, Normalized size: {}",
                 text.size(), final_normalized.size());

        return final_normalized;
    }

} // namespace inverted_index
