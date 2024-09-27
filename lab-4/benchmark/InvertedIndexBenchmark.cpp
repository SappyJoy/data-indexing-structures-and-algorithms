#include "Dataset.hpp"
#include "inverted_index/InvertedIndex.hpp"
#include "inverted_index/QueryProcessor.hpp"
#include "log/Logger.hpp"
#include <benchmark/benchmark.h>
#include <memory>
#include <string>
#include <vector>

using namespace inverted_index;

/**
 * @brief Fixture class to set up the benchmarking environment.
 */
class InvertedIndexBenchmark : public benchmark::Fixture {
  public:
    /**
     * @brief Setup function called before each benchmark.
     *
     * @param state The benchmark state.
     */
    void SetUp(const ::benchmark::State &state) override {
        Logger::getInstance().setLogLevel(LogLevel::ERROR);
        // Load the full dataset
        full_dataset = std::make_unique<Dataset>("data/trec/test.csv");
        all_documents = &full_dataset->getDocuments();

        // Initialize the inverted index
        index = std::make_unique<InvertedIndex>();
        query_processor = std::make_unique<QueryProcessor>(*index);
    }

    /**
     * @brief Teardown function called after each benchmark.
     *
     * @param state The benchmark state.
     */
    void TearDown(const ::benchmark::State &state) override {
        index.reset();
        query_processor.reset();
        full_dataset.reset();
    }

  protected:
    std::unique_ptr<Dataset> full_dataset;           ///< Pointer to the loaded dataset.
    const std::vector<Document> *all_documents;      ///< Pointer to all loaded documents.
    std::unique_ptr<InvertedIndex> index;            ///< Pointer to the inverted index.
    std::unique_ptr<QueryProcessor> query_processor; ///< Pointer to the query processor.
};

/**
 * @brief Benchmark for measuring the time to build the inverted index from scratch.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, IndexConstruction)(benchmark::State &state) {
    for (auto _ : state) {
        InvertedIndex temp_index;
        for (const auto &doc : *all_documents) {
            temp_index.addDocument(doc.doc_id, doc.text);
        }
        benchmark::DoNotOptimize(temp_index);
    }
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, IndexConstruction)->Unit(benchmark::kMillisecond);

/**
 * @brief Helper function to generate sample queries.
 *
 * @return A vector of sample queries covering simple, complex, and unary types.
 */
std::vector<std::string> generateSampleQueries() {
    return {// Simple Queries
            "hello AND dog", "cat OR fox", "world AND again",

            // Complex Queries with Multiple Operators and Parentheses
            "(hello OR fox) AND dog", "hello AND (world OR again)", "((hello AND world) OR (fox AND dog)) AND again",

            // Queries with Only Unary Operators
            "NOT dog", "NOT (hello OR fox)", "hello AND NOT dog",

            // Mixed Queries
            "hello OR (dog AND (world OR NOT fox))", "(hello AND NOT dog) OR (cat AND fox)"};
}

/**
 * @brief Benchmark for measuring query execution time across various query types.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, QueryExecution)(benchmark::State &state) {
    // Pre-build the index with all documents
    for (const auto &doc : *all_documents) {
        index->addDocument(doc.doc_id, doc.text);
    }

    // Generate sample queries
    std::vector<std::string> queries = generateSampleQueries();

    for (auto _ : state) {
        for (const auto &query : queries) {
            auto result = query_processor->executeQuery(query);
            benchmark::DoNotOptimize(result);
        }
    }
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, QueryExecution)->Unit(benchmark::kMicrosecond);

/**
 * @brief Benchmark for measuring the performance when adding documents incrementally.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, IncrementalIndexing)(benchmark::State &state) {
    for (auto _ : state) {
        InvertedIndex temp_index;
        for (const auto &doc : *all_documents) {
            temp_index.addDocument(doc.doc_id, doc.text);
        }
        benchmark::DoNotOptimize(temp_index);
    }
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, IncrementalIndexing)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark for measuring the performance of rebuilding the index after modifications.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, RebuildingIndex)(benchmark::State &state) {
    // Pre-build the index
    for (const auto &doc : *all_documents) {
        index->addDocument(doc.doc_id, doc.text);
    }

    for (auto _ : state) {
        InvertedIndex temp_index;
        // Simulate modifications: Remove all documents and re-add them
        // (Assuming InvertedIndex supports such operations)
        // For simplicity, we'll just rebuild the index
        for (const auto &doc : *all_documents) {
            temp_index.addDocument(doc.doc_id, doc.text);
        }
        benchmark::DoNotOptimize(temp_index);
    }
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, RebuildingIndex)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark for measuring peak memory consumption.
 * Note: Memory consumption is best measured using external tools like Valgrind's Massif.
 * This benchmark provides a placeholder to indicate where such measurements can be integrated.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, PeakMemoryConsumption)(benchmark::State &state) {
    // This benchmark is a placeholder.
    // Use external profiling tools like Valgrind's Massif to measure memory.
    for (auto _ : state) {
        InvertedIndex temp_index;
        for (const auto &doc : *all_documents) {
            temp_index.addDocument(doc.doc_id, doc.text);
        }
        benchmark::DoNotOptimize(temp_index);
    }
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, PeakMemoryConsumption)->Unit(benchmark::kMillisecond);

/**
 * @brief Benchmark for measuring the throughput: number of queries processed per second.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, QueryThroughput)(benchmark::State &state) {
    // Pre-build the index with all documents
    for (const auto &doc : *all_documents) {
        index->addDocument(doc.doc_id, doc.text);
    }

    // Generate sample queries
    std::vector<std::string> queries = generateSampleQueries();

    for (auto _ : state) {
        for (const auto &query : queries) {
            query_processor->executeQuery(query);
        }
    }
    state.SetItemsProcessed(state.iterations() * queries.size());
}
BENCHMARK_REGISTER_F(InvertedIndexBenchmark, QueryThroughput)->Unit(benchmark::kMillisecond);

/**
 * @brief Function to generate datasets of varying sizes (small, medium, large).
 *
 * @param full_documents Reference to all documents loaded.
 * @param size_factor Factor to scale the dataset size (1 for small, 10 for medium, etc.).
 * @return A vector of Document pointers representing the scaled dataset.
 */
std::vector<Document> generateScaledDataset(const std::vector<Document> &full_documents, int size_factor) {
    std::vector<Document> scaled;
    for (int i = 0; i < size_factor; ++i) {
        for (const auto &doc : full_documents) {
            scaled.emplace_back(Document{doc.doc_id + i * (int)full_documents.size(), doc.text});
        }
    }
    return scaled;
}

/**
 * @brief Benchmark for executing queries on small, medium, and large indexes.
 *
 * @param state The benchmark state.
 */
BENCHMARK_DEFINE_F(InvertedIndexBenchmark, QueryExecutionDifferentDataSizes)(benchmark::State &state) {
    int scale_factor = state.range(0);

    for (auto _ : state) {
        // Generate scaled dataset
        auto scaled_documents = generateScaledDataset(*all_documents, scale_factor);

        // Build the index
        InvertedIndex temp_index;
        for (const auto &doc : scaled_documents) {
            temp_index.addDocument(doc.doc_id, doc.text);
        }

        // Initialize a temporary QueryProcessor
        QueryProcessor temp_qp(temp_index);

        // Define a sample query
        std::string query = "(hello OR fox) AND dog";

        // Execute the query
        auto result = temp_qp.executeQuery(query);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK_REGISTER_F(InvertedIndexBenchmark, QueryExecutionDifferentDataSizes)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
