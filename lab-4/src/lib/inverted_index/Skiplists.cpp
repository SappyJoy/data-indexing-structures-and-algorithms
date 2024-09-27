#include "inverted_index/Skiplists.hpp"
#include "log/Logger.hpp"

namespace inverted_index {

void Skiplists::addSkipPointer(const std::string& term, const SkipPointer& skip_pointer) {
    skip_map_[term].emplace_back(skip_pointer);
}

const std::vector<SkipPointer>& Skiplists::getSkipPointers(const std::string& term) const {
    static const std::vector<SkipPointer> empty;
    auto it = skip_map_.find(term);
    if (it != skip_map_.end()) {
        return it->second;
    }
    return empty;
}

bool Skiplists::hasSkipPointers(const std::string& term) const {
    return skip_map_.find(term) != skip_map_.end();
}

void Skiplists::clear() {
    skip_map_.clear();
}

void Skiplists::buildSkipPointers(const std::string& term, const std::vector<uint8_t>& compressed_data) {
    LOG_DEBUG("Building skip pointers for term '{}'.", term);
    if (compressed_data.empty()) {
        LOG_WARNING("No compressed data provided for term '{}'. Skippointers not built.", term);
        return;
    }

    size_t current = 0;
    size_t data_size = compressed_data.size();

    std::vector<SkipPointer> term_skips;

    // Initialize current_doc_id to 0 before processing
    int current_doc_id = 0;

    while (current + 2 <= data_size) { // Ensure p and gap_count can be read
        uint8_t p = compressed_data[current];
        uint8_t gap_count = compressed_data[current + 1];
        LOG_DEBUG("Parsing block: p = {}, gap_count = {}", p, gap_count);
        current += 2;

        // Record the byte_offset where this block starts
        size_t block_start_offset = current - 2; // p and gap_count are part of the block

        // Read the gaps
        size_t bits_per_gap = p;
        size_t total_bits = bits_per_gap * gap_count;
        size_t bytes_needed = (total_bits + 7) / 8;

        if (current + bytes_needed > data_size) {
            LOG_ERROR("Insufficient data to read all gaps for the block.");
            throw std::invalid_argument("Compressed data corrupted or incomplete.");
        }

        // Decode the first gap to get the starting doc_id for the block
        if (gap_count < 1) {
            LOG_WARNING("Block with zero gaps encountered. Skipping.");
            continue;
        }

        // Decode the first gap
        uint64_t buffer = 0;
        uint8_t bits_in_buffer = 0;
        size_t bytes_read = 0;

        // Fill the buffer with p bits
        for (size_t i = 0; i < p && bytes_read < bytes_needed; ++i) {
            buffer |= static_cast<uint64_t>(compressed_data[current + bytes_read]) << bits_in_buffer;
            bits_in_buffer += 8;
            bytes_read += 1;
        }

        if (bits_in_buffer < bits_per_gap) {
            LOG_ERROR("Insufficient bits to decode the first gap in the block.");
            throw std::invalid_argument("Compressed data corrupted or incomplete.");
        }

        int first_gap = static_cast<int>(buffer & ((1ULL << p) - 1));
        current_doc_id += first_gap;
        LOG_DEBUG("Block starting doc_id: {}", current_doc_id);

        // Record the skip pointer at the start of the block
        SkipPointer skip_ptr = {current_doc_id, block_start_offset};
        term_skips.emplace_back(skip_ptr);
        LOG_DEBUG("Added skip pointer: doc_id = {}, byte_offset = {}", skip_ptr.doc_id, skip_ptr.byte_offset);

        // Advance the buffer by p bits
        buffer >>= p;
        bits_in_buffer -= p;

        // Skip the rest of the gaps in the block (not needed for building skip pointers)
        // Just update current_doc_id based on the remaining gaps
        for (size_t i = 1; i < gap_count; ++i) {
            // Extract next gap
            while (bits_in_buffer < bits_per_gap && bytes_read < bytes_needed) {
                buffer |= static_cast<uint64_t>(compressed_data[current + bytes_read]) << bits_in_buffer;
                bits_in_buffer += 8;
                bytes_read += 1;
            }

            if (bits_in_buffer < bits_per_gap) {
                LOG_ERROR("Insufficient bits to decode gap {} in block.", i);
                throw std::invalid_argument("Compressed data corrupted or incomplete.");
            }

            int gap = static_cast<int>(buffer & ((1ULL << bits_per_gap) - 1));
            current_doc_id += gap;

            buffer >>= bits_per_gap;
            bits_in_buffer -= bits_per_gap;
        }

        // Update current to skip the bytes used
        current += bytes_read;
    }

    // Assign the built skip pointers to the term
    skip_map_[term] = std::move(term_skips);

    LOG_INFO("Built {} skip pointers for term '{}'.", skip_map_[term].size(), term);
}

void Skiplists::addSkipPointers(const std::string& term, const std::vector<SkipPointer>& skips) {
    auto& term_skips = skip_map_[term];
    term_skips.insert(term_skips.end(), skips.begin(), skips.end());
}

} // namespace inverted_index
