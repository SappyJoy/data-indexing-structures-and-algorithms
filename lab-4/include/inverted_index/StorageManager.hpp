#ifndef INVERTED_INDEX_STORE_MANAGER_HPP
#define INVERTED_INDEX_STORE_MANAGER_HPP

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>


/**
 * @brief Manages the persistent storage of the inverted index, handling serialization and deserialization.
 */
namespace inverted_index {

class InvertedIndex;
class Skiplists;

class StorageManager {
public:
    /**
     * @brief Saves the entire inverted index to a binary file.
     *
     * @param index The InvertedIndex instance to serialize.
     * @param filepath The path to the file where the index will be saved.
     * @throws std::runtime_error If the file cannot be opened or writing fails.
     */
    static void saveIndex(const InvertedIndex& index, const std::string& filepath);

    /**
     * @brief Loads the inverted index from a binary file.
     *
     * @param index The InvertedIndex instance to populate.
     * @param filepath The path to the file from which the index will be loaded.
     * @throws std::runtime_error If the file cannot be opened, format is incorrect, or reading fails.
     */
    static void loadIndex(InvertedIndex& index, const std::string& filepath);

private:
    // Magic number to identify the file format
    static constexpr char MAGIC_NUMBER[4] = {'S', 'A', 'P', 'J'};
    // Version number for the file format
    static constexpr uint32_t VERSION = 1;

    /**
     * @brief Writes raw bytes to the output stream.
     *
     * @tparam T The type of data to write.
     * @param out The output stream.
     * @param data The data to write.
     */
    template<typename T>
    static void writeBinary(std::ostream& out, const T& data) {
        out.write(reinterpret_cast<const char*>(&data), sizeof(T));
        if (!out) {
            throw std::runtime_error("Failed to write binary data.");
        }
    }

    /**
     * @brief Reads raw bytes from the input stream.
     *
     * @tparam T The type of data to read.
     * @param in The input stream.
     * @param data The variable to store the read data.
     */
    template<typename T>
    static void readBinary(std::istream& in, T& data) {
        in.read(reinterpret_cast<char*>(&data), sizeof(T));
        if (!in) {
            throw std::runtime_error("Failed to read binary data.");
        }
    }

    /**
     * @brief Writes a string to the output stream with its length.
     *
     * @param out The output stream.
     * @param str The string to write.
     */
    static void writeString(std::ostream& out, const std::string& str);

    /**
     * @brief Reads a string from the input stream based on its length.
     *
     * @param in The input stream.
     * @param str The string to populate.
     */
    static void readString(std::istream& in, std::string& str);

    /**
     * @brief Writes a vector of bytes to the output stream.
     *
     * @param out The output stream.
     * @param data The vector of bytes to write.
     */
    static void writeBytes(std::ostream& out, const std::vector<uint8_t>& data);

/**
     * @brief Reads a specified number of bytes from the input stream into a vector.
     *
     * @param in The input stream.
     * @param data The vector to populate with read bytes.
     * @param size The exact number of bytes to read.
     * @throws std::runtime_error If the read operation fails or insufficient data is available.
     */
    static void readBytes(std::istream& in, std::vector<uint8_t>& data, size_t size);
};

} // namespace inverted_index

#endif
