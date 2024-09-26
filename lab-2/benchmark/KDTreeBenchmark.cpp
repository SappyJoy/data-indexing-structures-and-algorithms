#include "FashionMNIST.hpp"
#include "kdtree/Point.hpp"
#include "kdtree/indexes/FAISSIndex.hpp"
#include "kdtree/indexes/KDTreeIndex.hpp"
#include <benchmark/benchmark.h>
#include <iostream>
#include <vector>

using namespace kdtree;

// Global variable to hold the loaded dataset
static std::vector<Point> g_fashion_mnist_data;

// Benchmark fixture to load data once
struct KDTreeBenchmarkFixture : public benchmark::Fixture {
    void SetUp(const ::benchmark::State &state) {
        if (g_fashion_mnist_data.empty()) {
            try {
                std::cout << "Loading Fashion MNIST dataset...\n";
                g_fashion_mnist_data = LoadFashionMNIST("data/fashion-mnist/fashion-mnist_test.csv");
                std::cout << "Loaded " << g_fashion_mnist_data.size() << " points.\n";
            } catch (const std::exception &e) {
                std::cerr << "Error loading dataset: " << e.what() << "\n";
                std::exit(EXIT_FAILURE);
            }
        }
    }

    void TearDown(const ::benchmark::State &state) {}
};

// Register the fixture
BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, KDTreeIndex_Build)(benchmark::State &state) {
    for (auto _ : state) {
        KDTreeIndex tree;
        tree.build(g_fashion_mnist_data);
        benchmark::DoNotOptimize(tree);
    }
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, KDTreeIndex_Build)->Unit(benchmark::kMillisecond)->Iterations(10);

BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, FAISSIndex_Build)(benchmark::State &state) {
    for (auto _ : state) {
        FAISSIndex faiss_index;
        faiss_index.build(g_fashion_mnist_data);
        benchmark::DoNotOptimize(faiss_index);
    }
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, FAISSIndex_Build)->Unit(benchmark::kMillisecond)->Iterations(10);

// Helper function to generate random query points
std::vector<Point> GenerateRandomQueries(size_t num_queries, size_t dimension) {
    std::vector<Point> queries;
    queries.reserve(num_queries);
    for (size_t i = 0; i < num_queries; ++i) {
        std::vector<float> coords(dimension);
        for (size_t d = 0; d < dimension; ++d) {
            coords[d] = static_cast<float>(rand()) / RAND_MAX;
        }
        queries.emplace_back(coords);
    }
    return queries;
}

// Benchmark for KDTreeIndex Nearest Neighbors
BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, KDTreeIndex_NearestNeighbors)(benchmark::State &state) {
    KDTreeIndex tree;
    tree.build(g_fashion_mnist_data);

    // Generate a set of query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);

    for (auto _ : state) {
        for (const auto &query : queries) {
            auto neighbors = tree.nearest_neighbors(query, k);
            benchmark::DoNotOptimize(neighbors);
        }
    }

    state.SetComplexityN(num_queries * k);
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, KDTreeIndex_NearestNeighbors)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10)
    ->Complexity(benchmark::oN);

// Benchmark for FAISSIndex Nearest Neighbors
BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, FAISSIndex_NearestNeighbors)(benchmark::State &state) {
    FAISSIndex faiss_index;
    faiss_index.build(g_fashion_mnist_data);

    // Generate a set of query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);

    for (auto _ : state) {
        for (const auto &query : queries) {
            auto neighbors = faiss_index.nearest_neighbors(query, k);
            benchmark::DoNotOptimize(neighbors);
        }
    }

    state.SetComplexityN(num_queries * k);
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, FAISSIndex_NearestNeighbors)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10)
    ->Complexity(benchmark::oN);

// Benchmark for KDTreeIndex Range Search
BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, KDTreeIndex_RangeSearch)(benchmark::State &state) {
    KDTreeIndex tree;
    tree.build(g_fashion_mnist_data);

    // Generate a set of query points
    size_t num_queries = 100;
    double radius = 0.1; // Adjust radius as needed
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);

    for (auto _ : state) {
        for (const auto &query : queries) {
            auto results = tree.range_search(query, radius);
            benchmark::DoNotOptimize(results);
        }
    }

    state.SetComplexityN(num_queries);
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, KDTreeIndex_RangeSearch)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10)
    ->Complexity(benchmark::oN);

// Benchmark for FAISSIndex Range Search
BENCHMARK_DEFINE_F(KDTreeBenchmarkFixture, FAISSIndex_RangeSearch)(benchmark::State &state) {
    FAISSIndex faiss_index;
    faiss_index.build(g_fashion_mnist_data);

    // Generate a set of query points
    size_t num_queries = 100;
    double radius = 0.1; // Adjust radius as needed
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);

    for (auto _ : state) {
        for (const auto &query : queries) {
            auto results = faiss_index.range_search(query, radius);
            benchmark::DoNotOptimize(results);
        }
    }

    state.SetComplexityN(num_queries);
}

BENCHMARK_REGISTER_F(KDTreeBenchmarkFixture, FAISSIndex_RangeSearch)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10)
    ->Complexity(benchmark::oN);

BENCHMARK_MAIN();
