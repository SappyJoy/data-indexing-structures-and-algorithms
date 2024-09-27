#ifndef DATASET_HPP
#define DATASET_HPP

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Represents a single document in the dataset.
 */
struct Document {
    int doc_id;       ///< Unique identifier for the document.
    std::string text; ///< Textual content of the document.
};

/**
 * @brief Handles loading and managing the TREC dataset.
 */
class Dataset {
  public:
    /**
     * @brief Constructs a Dataset object and loads data from the specified CSV file.
     *
     * @param filepath Path to the CSV file containing the dataset.
     * @throws std::runtime_error If the file cannot be opened or parsing fails.
     */
    Dataset(const std::string &filepath) { loadDataset(filepath); }

    /**
     * @brief Retrieves all documents in the dataset.
     *
     * @return A constant reference to the vector of documents.
     */
    const std::vector<Document> &getDocuments() const { return documents_; }

  private:
    std::vector<Document> documents_; ///< Container for all loaded documents.

    /**
     * @brief Loads the dataset from a CSV file.
     *
     * @param filepath Path to the CSV file.
     * @throws std::runtime_error If the file cannot be opened or parsing fails.
     */
    void loadDataset(const std::string &filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open dataset file: " + filepath);
        }

        std::string line;
        bool is_header = true;
        int current_doc_id = 1;

        while (std::getline(file, line)) {
            if (is_header) {
                // Skip header line
                is_header = false;
                continue;
            }

            if (line.empty()) {
                continue; // Skip empty lines
            }

            // Parse CSV line
            std::vector<std::string> fields = parseCSVLine(line);
            if (fields.size() != 3) {
                throw std::runtime_error("Malformed CSV line: " + line);
            }

            // Extract the text field (third column)
            std::string text = fields[2];
            // Remove any surrounding quotes
            if (!text.empty() && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }

            // Normalize whitespace in text
            normalizeWhitespace(text);

            // Create Document and add to the list
            Document doc;
            doc.doc_id = current_doc_id++;
            doc.text = text;
            documents_.emplace_back(doc);
        }

        file.close();

        if (documents_.empty()) {
            throw std::runtime_error("No documents loaded from the dataset.");
        }
    }

    /**
     * @brief Parses a single CSV line into fields.
     *
     * @param line The CSV line to parse.
     * @return A vector of parsed fields.
     */
    std::vector<std::string> parseCSVLine(const std::string &line) const {
        std::vector<std::string> result;
        std::stringstream ss(line);
        std::string field;
        bool in_quotes = false;
        std::string current_field;

        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"' && (i == 0 || line[i - 1] != '\\')) {
                in_quotes = !in_quotes;
                continue; // Skip the quote character
            }

            if (c == ',' && !in_quotes) {
                result.emplace_back(current_field);
                current_field.clear();
            } else {
                current_field += c;
            }
        }
        result.emplace_back(current_field); // Add the last field

        return result;
    }

    /**
     * @brief Normalizes whitespace in a string by replacing multiple spaces with a single space.
     *
     * @param text The string to normalize.
     */
    void normalizeWhitespace(std::string &text) const {
        // Remove leading and trailing whitespace
        text.erase(text.begin(),
                   std::find_if(text.begin(), text.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        text.erase(std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
                   text.end());

        // Replace multiple spaces with a single space
        std::string::iterator new_end = std::unique(
            text.begin(), text.end(), [](char lhs, char rhs) { return std::isspace(lhs) && std::isspace(rhs); });
        text.erase(new_end, text.end());
    }
};

#endif // DATASET_HPP
