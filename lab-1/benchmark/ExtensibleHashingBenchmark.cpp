#include "Dataset.hpp"
#include "inverted_index/pForDelta.hpp"
#include "log/Logger.hpp"
#include <algorithm>
#include <benchmark/benchmark.h>
#include <memory>
#include <vector>

using namespace inverted_index;

static std::vector<int> generateScaledDocIDs(const std::vector<int> &base_doc_ids, int scale_factor) {
    std::vector<int> scaled_doc_ids;
    scaled_doc_ids.reserve(base_doc_ids.size() * scale_factor);
    int offset = 0;
    for (int i = 0; i < scale_factor; ++i) {
        for (const auto &id : base_doc_ids) {
            scaled_doc_ids.emplace_back(id + offset);
        }
        offset += 1000000; // Ensure unique doc_ids
    }
    return scaled_doc_ids;
}

/**
 * @brief Fixture class to set up the benchmarking environment for pForDelta.
 */
class PForDeltaBenchmark : public benchmark::Fixture {
  public:
    /**
     * @brief Setup function called before each benchmark.
     *
     * @param state The benchmark state.
     */
    void SetUp(const ::benchmark::State &state) override {
        Logger::getInstance().setLogLevel(LogLevel::ERROR);
        // Load the dataset
        dataset = std::make_unique<Dataset>("data/trec/test.csv");
        all_documents = &dataset->getDocuments();

        // Prepare document ID lists for benchmarking
        // For simplicity, we'll use the doc_ids directly
        // Ensure the doc_ids are sorted
        doc_ids = std::vector<int>();
        for (const auto &doc : *all_documents) {
            doc_ids.emplace_back(doc.doc_id);
        }
        std::sort(doc_ids.begin(), doc_ids.end());

        // Optionally, generate larger datasets by repeating existing doc_ids
        // Uncomment the following lines to generate larger datasets
        /*
        int repeat_factor = 10; // Adjust as needed
        std::vector<int> larger_doc_ids;
        for (int i = 0; i < repeat_factor; ++i) {
            for (const auto& id : doc_ids) {
                larger_doc_ids.emplace_back(id + i * 10000); // Offset to keep doc_ids unique
            }
        }
        doc_ids = larger_doc_ids;
        */
    }

    /**
     * @brief Teardown function called after each benchmark.
     *
     * @param state The benchmark state.
     */
    void TearDown(const ::benchmark::State &state) override { dataset.reset(); }

  protected:
    std::unique_ptr<Dataset> dataset;           ///< Pointer to the loaded dataset.
    const std::vector<Document> *all_documents; ///< Pointer to all loaded documents.
    std::vector<int> doc_ids;                   ///< Vector of document IDs for benchmarking.
};

/**
 * @brief Benchmark for measuring pForDelta encoding time and compression rate.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(PForDeltaBenchmark, Encode)(benchmark::State &state) {
    for (auto _ : state) {
        auto compressed = PForDelta::encode(doc_ids);
        benchmark::DoNotOptimize(compressed);
    }
    // Calculate compression rate
    // Original size: number of integers * sizeof(int)
    size_t original_size = doc_ids.size() * sizeof(int);
    // Compressed size: size of compressed vector
    size_t compressed_size = PForDelta::encode(doc_ids).size();
    // Compression rate
    double compression_rate = static_cast<double>(original_size) / compressed_size;
    state.counters["CompressionRate"] = compression_rate;
}
BENCHMARK_REGISTER_F(PForDeltaBenchmark, Encode)->Unit(benchmark::kMicrosecond);

/**
 * @brief Benchmark for measuring pForDelta decoding time.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(PForDeltaBenchmark, Decode)(benchmark::State &state) {
    // Pre-encode the data
    auto compressed = PForDelta::encode(doc_ids);
    for (auto _ : state) {
        auto decoded = PForDelta::decode(compressed);
        benchmark::DoNotOptimize(decoded);
    }
}
BENCHMARK_REGISTER_F(PForDeltaBenchmark, Decode)->Unit(benchmark::kMicrosecond);

/**
 * @brief Combined benchmark for encoding and then decoding, measuring total time and compression rate.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(PForDeltaBenchmark, EncodeDecode)(benchmark::State &state) {
    for (auto _ : state) {
        auto compressed = PForDelta::encode(doc_ids);
        auto decoded = PForDelta::decode(compressed);
        benchmark::DoNotOptimize(decoded);
    }
    // Calculate compression rate
    size_t original_size = doc_ids.size() * sizeof(int);
    size_t compressed_size = PForDelta::encode(doc_ids).size();
    double compression_rate = static_cast<double>(original_size) / compressed_size;
    state.counters["CompressionRate"] = compression_rate;
}
BENCHMARK_REGISTER_F(PForDeltaBenchmark, EncodeDecode)->Unit(benchmark::kMicrosecond);

/**
 * @brief Benchmark for encoding, decoding, and compression rate across various data sizes.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(PForDeltaBenchmark, EncodeDecode_VariousSizes)(benchmark::State &state) {
    int scale_factor = state.range(0); // Scale factor passed via Args
    auto scaled_doc_ids = generateScaledDocIDs(doc_ids, scale_factor);

    for (auto _ : state) {
        auto compressed = PForDelta::encode(scaled_doc_ids);
        auto decoded = PForDelta::decode(compressed);
        benchmark::DoNotOptimize(decoded);
    }

    // Calculate compression rate
    size_t original_size = scaled_doc_ids.size() * sizeof(int);
    size_t compressed_size = PForDelta::encode(scaled_doc_ids).size();
    double compression_rate = static_cast<double>(original_size) / compressed_size;
    state.counters["CompressionRate"] = compression_rate;
}
BENCHMARK_REGISTER_F(PForDeltaBenchmark, EncodeDecode_VariousSizes)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Unit(benchmark::kMicrosecond);

/**
 * @brief Main function for Google Benchmark.
 */
BENCHMARK_MAIN();
