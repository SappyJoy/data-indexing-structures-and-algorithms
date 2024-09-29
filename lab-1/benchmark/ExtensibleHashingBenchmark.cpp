#include "ehash/ExtensibleHashing.hpp"
#include "TestMessage.pb.h"
#include <benchmark/benchmark.h>
#include <filesystem>
#include <memory>
#include <random>

namespace ehash {
using namespace ehash::proto;

// Temporary benchmark directory for buckets
const std::string BENCHMARK_DIR = "benchmark_buckets";

// Helper function to create a TestMessage with a specific ID
std::unique_ptr<TestMessage> createTestMessage(int id) {
    auto message = std::make_unique<TestMessage>();
    message->set_id(id);
    return message;
}

// Fixture for setting up and tearing down the benchmark environment
class ExtensibleHashingBenchmark : public benchmark::Fixture {
  protected:
    void SetUp(const ::benchmark::State& state) override {
        // Ensure the benchmark directory is clean
        if (std::filesystem::exists(BENCHMARK_DIR)) {
            std::filesystem::remove_all(BENCHMARK_DIR);
        }
        std::filesystem::create_directory(BENCHMARK_DIR);

        // Initialize ExtensibleHashing with a configurable bucket size and initial global depth
        bucketSize = state.range(0); // Bucket size passed as a range argument
        initialGlobalDepth = 3;      // You can adjust this as needed
        hashTable = std::make_unique<ExtensibleHashing<TestMessage>>(BENCHMARK_DIR, bucketSize, initialGlobalDepth);

        // Prepare a large number of TestMessages for benchmarking
        totalEntries = state.range(1); // Total entries to benchmark
        for (size_t i = 0; i < totalEntries; ++i) {
            entries.push_back(createTestMessage(i));
        }

        // Precompute hash values to avoid measuring hash computation time in some benchmarks
        for (const auto& entry : entries) {
            serializedKeys.push_back(entry->SerializeAsString());
        }
    }

    void TearDown(const ::benchmark::State& state) override {
        // Clean up the benchmark directory after tests
        if (std::filesystem::exists(BENCHMARK_DIR)) {
            std::filesystem::remove_all(BENCHMARK_DIR);
        }
    }

    size_t bucketSize;
    size_t initialGlobalDepth;
    size_t totalEntries;
    std::unique_ptr<ExtensibleHashing<TestMessage>> hashTable;
    std::vector<std::unique_ptr<TestMessage>> entries;
    std::vector<std::string> serializedKeys;
};

// Benchmark: Adding entries to the hash table
BENCHMARK_DEFINE_F(ExtensibleHashingBenchmark, AddEntries)(benchmark::State& state) {
    for (auto _ : state) {
        for (auto& entry : entries) {
            // Clone the entry to ensure each add operation has a unique object
            auto entryClone = createTestMessage(entry->id());
            hashTable->addEntry(std::move(entryClone));
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * totalEntries);
}

// Register the benchmark with different bucket sizes and entry counts
BENCHMARK_REGISTER_F(ExtensibleHashingBenchmark, AddEntries)
    ->Args({4096, 1000})    // Bucket size: 4KB, Entries: 1,000
    ->Args({8192, 5000})    // Bucket size: 8KB, Entries: 5,000
    ->Args({16384, 10000})  // Bucket size: 16KB, Entries: 10,000
    ->Unit(benchmark::kMillisecond);

// Benchmark: Retrieving entries from the hash table
BENCHMARK_DEFINE_F(ExtensibleHashingBenchmark, RetrieveEntries)(benchmark::State& state) {
    // First, add all entries to the hash table
    for (auto& entry : entries) {
        auto entryClone = createTestMessage(entry->id());
        hashTable->addEntry(std::move(entryClone));
    }

    for (auto _ : state) {
        for (size_t i = 0; i < totalEntries; ++i) {
            size_t hashValue = hashTable->hashKey(serializedKeys[i]);
            hashTable->getEntry(hashValue);
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * totalEntries);
}

BENCHMARK_REGISTER_F(ExtensibleHashingBenchmark, RetrieveEntries)
    ->Args({4096, 1000})
    ->Args({8192, 5000})
    ->Args({16384, 10000})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Updating entries in the hash table
BENCHMARK_DEFINE_F(ExtensibleHashingBenchmark, UpdateEntries)(benchmark::State& state) {
    // First, add all entries to the hash table
    for (auto& entry : entries) {
        auto entryClone = createTestMessage(entry->id());
        hashTable->addEntry(std::move(entryClone));
    }

    for (auto _ : state) {
        for (size_t i = 0; i < totalEntries; ++i) {
            // Create an updated entry with the same ID
            auto updatedEntry = createTestMessage(entries[i]->id());
            hashTable->addEntry(std::move(updatedEntry));
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * totalEntries);
}

BENCHMARK_REGISTER_F(ExtensibleHashingBenchmark, UpdateEntries)
    ->Args({4096, 1000})
    ->Args({8192, 5000})
    ->Args({16384, 10000})
    ->Unit(benchmark::kMillisecond);

// Benchmark: Handling bucket splits during additions
BENCHMARK_DEFINE_F(ExtensibleHashingBenchmark, HandleBucketSplits)(benchmark::State& state) {
    // Configure a small bucket size to force frequent splits
    bucketSize = state.range(0);
    hashTable = std::make_unique<ExtensibleHashing<TestMessage>>(BENCHMARK_DIR, bucketSize, initialGlobalDepth);

    for (auto _ : state) {
        for (auto& entry : entries) {
            auto entryClone = createTestMessage(entry->id());
            hashTable->addEntry(std::move(entryClone));
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * totalEntries);
}

BENCHMARK_REGISTER_F(ExtensibleHashingBenchmark, HandleBucketSplits)
    ->Args({1024, 1000})    // Small bucket size: 1KB, Entries: 1,000
    ->Args({2048, 5000})    // Bucket size: 2KB, Entries: 5,000
    ->Unit(benchmark::kMillisecond);

// Main function to run the benchmarks

} // namespace ehash

BENCHMARK_MAIN();

