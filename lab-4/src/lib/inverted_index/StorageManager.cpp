#include "inverted_index/StorageManager.hpp"
#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/Skiplists.hpp"
#include "log/Logger.hpp"

#include <cstring> // For memcpy
#include <fstream>

namespace inverted_index {

void StorageManager::saveIndex(const InvertedIndex &index, const std::string &filepath) {
    LOG_DEBUG("Saving inverted index to file '{}'.", filepath);

    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }

    // Write Header
    out.write(MAGIC_NUMBER, sizeof(MAGIC_NUMBER));
    if (!out) {
        throw std::runtime_error("Failed to write magic number.");
    }

    writeBinary(out, VERSION);
    LOG_DEBUG("Written magic number and version.");

    // Retrieve the index map and Skiplists
    const auto &index_map = index.getIndexMap();
    const Skiplists &skiplists = index.getSkiplists();

    // Write number of terms
    uint32_t num_terms = static_cast<uint32_t>(index_map.size());
    writeBinary(out, num_terms);
    LOG_DEBUG("Written number of terms: {}", num_terms);

    // Iterate through each term
    for (const auto &[term, compressed_posting] : index_map) {
        // Write term length and term string
        writeString(out, term);

        // Write posting list size and data
        uint32_t posting_size = static_cast<uint32_t>(compressed_posting.size());
        writeBinary(out, posting_size);
        writeBytes(out, compressed_posting);
        LOG_DEBUG("Written posting list for term '{}', size: {}", term, posting_size);

        // Retrieve skip pointers for the term
        const auto &skips = skiplists.getSkipPointers(term);
        uint32_t num_skips = static_cast<uint32_t>(skips.size());
        writeBinary(out, num_skips);
        LOG_DEBUG("Written number of skip pointers for term '{}': {}", term, num_skips);

        // Write each skip pointer
        for (const auto &skip : skips) {
            int32_t doc_id = static_cast<int32_t>(skip.doc_id);
            uint64_t byte_offset = static_cast<uint64_t>(skip.byte_offset);
            writeBinary(out, doc_id);
            writeBinary(out, byte_offset);
            LOG_DEBUG("Written skip pointer: doc_id = {}, byte_offset = {}", doc_id, byte_offset);
        }
    }

    out.close();
    LOG_INFO("Inverted index successfully saved to '{}'.", filepath);
}

void StorageManager::loadIndex(InvertedIndex &index, const std::string &filepath) {
    LOG_DEBUG("Loading inverted index from file '{}'.", filepath);

    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filepath);
    }

    // Read and verify Header
    char magic[4];
    in.read(magic, sizeof(magic));
    if (!in) {
        throw std::runtime_error("Failed to read magic number.");
    }

    if (std::memcmp(magic, MAGIC_NUMBER, sizeof(magic)) != 0) {
        throw std::runtime_error("Invalid magic number. File format not recognized.");
    }

    uint32_t file_version;
    readBinary(in, file_version);
    LOG_DEBUG("Read file version: {}", file_version);

    if (file_version != VERSION) {
        throw std::runtime_error("Unsupported file version: " + std::to_string(file_version));
    }

    // Read number of terms
    uint32_t num_terms;
    readBinary(in, num_terms);
    LOG_DEBUG("Number of terms to load: {}", num_terms);

    // Iterate through each term
    for (uint32_t i = 0; i < num_terms; ++i) {
        // Read term length and term string
        std::string term;
        readString(in, term);
        LOG_DEBUG("Loading term '{}'.", term);

        // Read posting list size and data
        uint32_t posting_size;
        readBinary(in, posting_size);
        std::vector<uint8_t> compressed_posting;
        readBytes(in, compressed_posting, posting_size);
        LOG_DEBUG("Loaded posting list for term '{}', size: {}", term, posting_size);

        // Read number of skip pointers
        uint32_t num_skips;
        readBinary(in, num_skips);
        LOG_DEBUG("Number of skip pointers for term '{}': {}", term, num_skips);

        std::vector<SkipPointer> skips;
        skips.reserve(num_skips);

        // Read each skip pointer
        for (uint32_t j = 0; j < num_skips; ++j) {
            int32_t doc_id;
            uint64_t byte_offset;
            readBinary(in, doc_id);
            readBinary(in, byte_offset);
            skips.emplace_back(SkipPointer{static_cast<int>(doc_id), static_cast<size_t>(byte_offset)});
            LOG_DEBUG("Loaded skip pointer: doc_id = {}, byte_offset = {}", doc_id, byte_offset);
        }

        // Insert the term into the InvertedIndex
        index.insertTerm(term, compressed_posting);
        index.insertSkips(term, skips);
        LOG_DEBUG("Inserted term '{}' with posting list and skip pointers.", term);
    }

    in.close();
    LOG_INFO("Inverted index successfully loaded from '{}'.", filepath);
}

void StorageManager::writeString(std::ostream &out, const std::string &str) {
    uint32_t length = static_cast<uint32_t>(str.size());
    writeBinary(out, length);
    out.write(str.c_str(), length);
    if (!out) {
        throw std::runtime_error("Failed to write string data.");
    }
    LOG_DEBUG("Written string of length {}.", length);
}

void StorageManager::readString(std::istream &in, std::string &str) {
    uint32_t length;
    readBinary(in, length);
    LOG_DEBUG("Reading string of length {}.", length);
    str.resize(length);
    if (length > 0) {
        in.read(&str[0], length);
        if (!in) {
            throw std::runtime_error("Failed to read string data.");
        }
    }
}

void StorageManager::writeBytes(std::ostream &out, const std::vector<uint8_t> &data) {
    if (!data.empty()) {
        out.write(reinterpret_cast<const char *>(data.data()), data.size());
        if (!out) {
            throw std::runtime_error("Failed to write byte data.");
        }
        LOG_DEBUG("Written {} bytes of compressed posting list.", data.size());
    }
}

void StorageManager::readBytes(std::istream &in, std::vector<uint8_t> &data, size_t size) {
    data.resize(size);
    if (size > 0) {
        in.read(reinterpret_cast<char *>(data.data()), size);
        if (static_cast<size_t>(in.gcount()) != size) {
            throw std::runtime_error("Failed to read the expected number of bytes.");
        }
        LOG_DEBUG("Read {} bytes of data.", size);
    }
}

} // namespace inverted_index
