#include "benchmark/benchmark.h"
#include "common/Utils.hpp"
#include <fcntl.h>

using namespace ehash;

static void BM_Print(benchmark::State &state) {
    for (auto _ : state) {
        common::print("Hello, World");
    }
}

BENCHMARK(BM_Print);

BENCHMARK_MAIN();
