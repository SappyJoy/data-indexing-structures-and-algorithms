#include "ehash/ExtensibleHashing.hpp"
#include <benchmark/benchmark.h>

using namespace ehash;

// Benchmark for inserting keys into the hash table
static void BM_Insert(benchmark::State &state) {
    ExtensibleHashing hash_table(2);
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            hash_table.insert(i, i * 10);
        }
    }
}
BENCHMARK(BM_Insert)->Range(8, 8 << 10); // Test with varying sizes

// Benchmark for searching keys in the hash table
static void BM_Search(benchmark::State &state) {
    ExtensibleHashing hash_table(2);
    for (int i = 0; i < state.range(0); ++i) {
        hash_table.insert(i, i * 10);
    }

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            benchmark::DoNotOptimize(hash_table.search(i));
        }
    }
}
BENCHMARK(BM_Search)->Range(8, 8 << 10);

// Benchmark for removing keys from the hash table
static void BM_Remove(benchmark::State &state) {
    ExtensibleHashing hash_table(2);
    for (int i = 0; i < state.range(0); ++i) {
        hash_table.insert(i, i * 10);
    }

    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            hash_table.remove(i);
        }
    }
}
BENCHMARK(BM_Remove)->Range(8, 8 << 10);

BENCHMARK_MAIN();
