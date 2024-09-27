#include "inverted_index/pForDelta.hpp"
#include "log/Logger.hpp"

#include <cmath>
#include <cstring>

namespace inverted_index {

/**
 * @brief Helper function to calculate the optimal number of bits (p) needed to represent the maximum value in a block.
 *
 * @param max_val The maximum gap value in the block.
 * @return The number of bits required to represent max_val.
 */
static uint8_t calculate_bits(int max_val) {
    if (max_val == 0)
        return 1;
    return static_cast<uint8_t>(std::ceil(std::log2(max_val + 1)));
}

std::vector<uint8_t> PForDelta::encode(const std::vector<int> &doc_ids) {
    LOG_DEBUG("Starting pForDelta encoding.");
    if (doc_ids.empty()) {
        LOG_WARNING("Empty document ID list provided for encoding.");
        return {};
    }

    // Step 1: Delta Encoding
    std::vector<int> gaps;
    gaps.reserve(doc_ids.size());
    gaps.push_back(doc_ids[0]); // First ID is stored as is
    for (size_t i = 1; i < doc_ids.size(); ++i) {
        int gap = doc_ids[i] - doc_ids[i - 1];
        if (gap <= 0) {
            LOG_ERROR("Document IDs must be strictly increasing. Found gap {} at position {}.", gap, i);
            throw std::invalid_argument("Document IDs are not sorted or contain duplicates.");
        }
        gaps.push_back(gap);
    }

    LOG_DEBUG("Delta encoding completed. Total gaps: {}", gaps.size());

    // Step 2: Block Processing
    const size_t block_size = 128; // Number of integers per block
    size_t num_blocks = (gaps.size() + block_size - 1) / block_size;

    std::vector<uint8_t> compressed;
    // Reserve more accurately: for each block, at least 1 byte for p, 1 byte for count, and bit-packed data
    compressed.reserve((1 + 1 + 1) * num_blocks); // Simplistic estimation

    size_t current = 0;
    for (size_t b = 0; b < num_blocks; ++b) {
        size_t block_start = current;
        size_t block_end = std::min(current + block_size, gaps.size());
        size_t actual_block_size = block_end - block_start;

        // Find the maximum gap in the block to determine the number of bits (p)
        int max_gap = 0;
        for (size_t i = block_start; i < block_end; ++i) {
            if (gaps[i] > max_gap) {
                max_gap = gaps[i];
            }
        }

        uint8_t p = calculate_bits(max_gap);
        LOG_DEBUG("Block {}: p = {}, max_gap = {}", b, p, max_gap);

        // Store p (number of bits) and the number of gaps in the block
        compressed.push_back(p);
        compressed.push_back(static_cast<uint8_t>(actual_block_size));

        // Pack the gaps using p bits each
        uint64_t buffer = 0;
        uint8_t bits_filled = 0;

        for (size_t i = block_start; i < block_end; ++i) {
            buffer |= static_cast<uint64_t>(gaps[i]) << bits_filled;
            bits_filled += p;

            while (bits_filled >= 8) {
                uint8_t byte = static_cast<uint8_t>(buffer & 0xFF);
                compressed.push_back(byte);
                buffer >>= 8;
                bits_filled -= 8;
            }
        }

        // Handle remaining bits in buffer
        if (bits_filled > 0) {
            uint8_t byte = static_cast<uint8_t>(buffer & 0xFF);
            compressed.push_back(byte);
        }

        current += actual_block_size;
    }

    LOG_INFO("pForDelta encoding completed. Original size: {} bytes, Compressed size: {} bytes.",
             gaps.size() * sizeof(int), compressed.size());

    return compressed;
}

std::vector<int> PForDelta::decode(const std::vector<uint8_t> &compressed_data) {
    LOG_DEBUG("Starting pForDelta decoding.");
    if (compressed_data.empty()) {
        LOG_WARNING("Empty compressed data provided for decoding.");
        return {};
    }

    std::vector<int> gaps;
    // Estimate the number of gaps based on compressed data size and block size
    // Not necessary, just reserve capacity
    gaps.reserve(compressed_data.size());

    size_t current = 0;
    size_t data_size = compressed_data.size();

    while (current < data_size) {
        if (current + 2 > data_size) {
            LOG_ERROR("Insufficient data to read p and gap count.");
            throw std::invalid_argument("Compressed data corrupted or incomplete.");
        }

        // Read p and gap_count
        uint8_t p = compressed_data[current];
        uint8_t gap_count = compressed_data[current + 1];
        LOG_DEBUG("Decoding block: p = {}, gap_count = {}", p, gap_count);
        current += 2;

        if (p == 0) {
            LOG_ERROR("Invalid bit width p = 0 encountered during decoding.");
            throw std::invalid_argument("Invalid bit width p = 0.");
        }

        // Read the bit-packed gaps
        size_t bits_per_gap = p;
        size_t total_bits = bits_per_gap * gap_count;
        size_t bytes_needed = (total_bits + 7) / 8;

        if (current + bytes_needed > data_size) {
            LOG_ERROR("Insufficient data to read all gaps for the block.");
            throw std::invalid_argument("Compressed data corrupted or incomplete.");
        }

        uint64_t buffer = 0;
        uint8_t bits_in_buffer = 0;
        size_t bytes_read = 0;

        for (size_t i = 0; i < gap_count; ++i) {
            // Ensure buffer has enough bits
            while (bits_in_buffer < bits_per_gap && bytes_read < bytes_needed) {
                buffer |= static_cast<uint64_t>(compressed_data[current + bytes_read]) << bits_in_buffer;
                bits_in_buffer += 8;
                bytes_read += 1;
            }

            if (bits_in_buffer < bits_per_gap) {
                LOG_ERROR("Insufficient bits to decode integer {} in block.", i);
                throw std::invalid_argument("Insufficient bits in compressed data.");
            }

            // Extract p bits
            int gap = static_cast<int>(buffer & ((1ULL << bits_per_gap) - 1));
            gaps.push_back(gap);
            buffer >>= bits_per_gap;
            bits_in_buffer -= bits_per_gap;
        }

        current += bytes_needed;
    }

    LOG_DEBUG("Delta decoding completed. Total gaps: {}", gaps.size());

    // Step 3: Reconstruct the original document IDs from gaps
    std::vector<int> doc_ids;
    doc_ids.reserve(gaps.size());

    if (gaps.empty()) {
        return doc_ids;
    }

    doc_ids.push_back(gaps[0]);
    for (size_t i = 1; i < gaps.size(); ++i) {
        doc_ids.push_back(doc_ids.back() + gaps[i]);
    }

    LOG_INFO("pForDelta decoding completed. Original document IDs size: {}, Decoded size: {}.", gaps.size(),
             doc_ids.size());

    return doc_ids;
}

} // namespace inverted_index
