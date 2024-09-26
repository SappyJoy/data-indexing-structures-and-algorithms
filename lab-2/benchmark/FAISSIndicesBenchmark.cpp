#include "FashionMNIST.hpp"
#include "kdtree/Point.hpp"
#include <benchmark/benchmark.h>
#include <cstdlib> // For rand()
#include <faiss/Clustering.h>
#include <faiss/IndexFlat.h>
#include <faiss/IndexHNSW.h>
#include <faiss/IndexIDMap.h>
#include <faiss/IndexIVFFlat.h>
#include <faiss/IndexPQ.h>
#include <faiss/IndexPreTransform.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace kdtree;

// Global variable to hold the loaded dataset
static std::vector<Point> g_fashion_mnist_data;

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

// Benchmark fixture to load data once
struct FAISSIndicesBenchmarkFixture : public benchmark::Fixture {
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

// Benchmark for FAISS IndexFlatL2 build time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexFlatL2_Build)(benchmark::State &state) {
    for (auto _ : state) {
        faiss::IndexFlatL2 index(g_fashion_mnist_data[0].dimension());
        std::vector<float> data;
        data.reserve(g_fashion_mnist_data.size() * 784);
        for (const auto &point : g_fashion_mnist_data) {
            const std::vector<float> &coords = point.coordinates();
            data.insert(data.end(), coords.begin(), coords.end());
        }
        index.add(g_fashion_mnist_data.size(), data.data());
        benchmark::DoNotOptimize(index);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexFlatL2_Build)->Unit(benchmark::kSecond)->Iterations(10);

// Benchmark for FAISS IndexFlatL2 search time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexFlatL2_Search)(benchmark::State &state) {
    faiss::IndexFlatL2 index(g_fashion_mnist_data[0].dimension());
    std::vector<float> data;
    data.reserve(g_fashion_mnist_data.size() * 784);
    for (const auto &point : g_fashion_mnist_data) {
        const std::vector<float> &coords = point.coordinates();
        data.insert(data.end(), coords.begin(), coords.end());
    }
    index.add(g_fashion_mnist_data.size(), data.data());

    // Generate a set of query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);
    std::vector<float> query_data;
    query_data.reserve(num_queries * 784);
    for (const auto &query : queries) {
        const std::vector<float> &coords = query.coordinates();
        query_data.insert(query_data.end(), coords.begin(), coords.end());
    }

    for (auto _ : state) {
        std::vector<faiss::idx_t> labels(k * num_queries);
        std::vector<float> distances(k * num_queries);
        index.search(num_queries, query_data.data(), k, distances.data(), labels.data());
        benchmark::DoNotOptimize(labels);
        benchmark::DoNotOptimize(distances);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexFlatL2_Search)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10);

// Benchmark for FAISS IndexIVFFlat build time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexIVFFlat_Build)(benchmark::State &state) {
    for (auto _ : state) {
        size_t dim = g_fashion_mnist_data[0].dimension();
        faiss::IndexFlatL2 quantizer(dim);
        size_t nlist = 100; // number of clusters
        faiss::IndexIVFFlat index(&quantizer, dim, nlist, faiss::METRIC_L2);

        // Prepare data
        std::vector<float> data;
        data.reserve(g_fashion_mnist_data.size() * 784);
        for (const auto &point : g_fashion_mnist_data) {
            const std::vector<float> &coords = point.coordinates();
            data.insert(data.end(), coords.begin(), coords.end());
        }

        // Train the index
        index.train(g_fashion_mnist_data.size(), data.data());

        // Add the data to the index
        index.add(g_fashion_mnist_data.size(), data.data());

        benchmark::DoNotOptimize(index);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexIVFFlat_Build)->Unit(benchmark::kSecond)->Iterations(10);

// Benchmark for FAISS IndexIVFFlat search time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexIVFFlat_Search)(benchmark::State &state) {
    size_t dim = g_fashion_mnist_data[0].dimension();
    size_t nlist = 100;
    faiss::IndexFlatL2 quantizer(dim);
    faiss::IndexIVFFlat index(&quantizer, dim, nlist, faiss::METRIC_L2);

    // Prepare data
    std::vector<float> data;
    data.reserve(g_fashion_mnist_data.size() * 784);
    for (const auto &point : g_fashion_mnist_data) {
        const std::vector<float> &coords = point.coordinates();
        data.insert(data.end(), coords.begin(), coords.end());
    }

    // Train the index
    index.train(g_fashion_mnist_data.size(), data.data());

    // Add data to the index
    index.add(g_fashion_mnist_data.size(), data.data());

    // Generate query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);
    std::vector<float> query_data;
    query_data.reserve(num_queries * 784);
    for (const auto &query : queries) {
        const std::vector<float> &coords = query.coordinates();
        query_data.insert(query_data.end(), coords.begin(), coords.end());
    }

    for (auto _ : state) {
        std::vector<faiss::idx_t> labels(k * num_queries);
        std::vector<float> distances(k * num_queries);
        index.search(num_queries, query_data.data(), k, distances.data(), labels.data());
        benchmark::DoNotOptimize(labels);
        benchmark::DoNotOptimize(distances);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexIVFFlat_Search)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10);

// Benchmark for FAISS IndexHNSWFlat build time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexHNSWFlat_Build)(benchmark::State &state) {
    for (auto _ : state) {
        size_t dim = g_fashion_mnist_data[0].dimension();
        int M = 32; // HNSW parameter
        faiss::IndexHNSWFlat index(dim, M);

        std::vector<float> data;
        data.reserve(g_fashion_mnist_data.size() * 784);
        for (const auto &point : g_fashion_mnist_data) {
            const std::vector<float> &coords = point.coordinates();
            data.insert(data.end(), coords.begin(), coords.end());
        }

        index.hnsw.efConstruction = 40; // HNSW parameter
        index.add(g_fashion_mnist_data.size(), data.data());

        benchmark::DoNotOptimize(index);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexHNSWFlat_Build)->Unit(benchmark::kSecond)->Iterations(10);

// Benchmark for FAISS IndexHNSWFlat search time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexHNSWFlat_Search)(benchmark::State &state) {
    size_t dim = g_fashion_mnist_data[0].dimension();
    int M = 32;
    faiss::IndexHNSWFlat index(dim, M);
    index.hnsw.efConstruction = 40;

    std::vector<float> data;
    data.reserve(g_fashion_mnist_data.size() * 784);
    for (const auto &point : g_fashion_mnist_data) {
        const std::vector<float> &coords = point.coordinates();
        data.insert(data.end(), coords.begin(), coords.end());
    }

    index.add(g_fashion_mnist_data.size(), data.data());

    // Generate query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);
    std::vector<float> query_data;
    query_data.reserve(num_queries * 784);
    for (const auto &query : queries) {
        const std::vector<float> &coords = query.coordinates();
        query_data.insert(query_data.end(), coords.begin(), coords.end());
    }

    index.hnsw.efSearch = 20; // HNSW search parameter

    for (auto _ : state) {
        std::vector<faiss::idx_t> labels(k * num_queries);
        std::vector<float> distances(k * num_queries);
        index.search(num_queries, query_data.data(), k, distances.data(), labels.data());
        benchmark::DoNotOptimize(labels);
        benchmark::DoNotOptimize(distances);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexHNSWFlat_Search)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10);

// Benchmark for FAISS IndexPQ build time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexPQ_Build)(benchmark::State &state) {
    for (auto _ : state) {
        size_t dim = g_fashion_mnist_data[0].dimension();
        size_t nbits = 8;
        // size_t ncentroids = 256; // Unused in this context, consider removing or using it
        size_t m = 16; // number of subquantizers

        faiss::IndexFlatL2 quantizer(dim);
        faiss::IndexPQ index(dim, m, nbits);

        // Prepare data
        std::vector<float> data;
        data.reserve(g_fashion_mnist_data.size() * 784);
        for (const auto &point : g_fashion_mnist_data) {
            const std::vector<float> &coords = point.coordinates();
            data.insert(data.end(), coords.begin(), coords.end());
        }

        // Train the index
        index.train(g_fashion_mnist_data.size(), data.data());

        // Add the data to the index
        index.add(g_fashion_mnist_data.size(), data.data());

        benchmark::DoNotOptimize(index);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexPQ_Build)->Unit(benchmark::kSecond)->Iterations(10);

// Benchmark for FAISS IndexPQ search time
BENCHMARK_DEFINE_F(FAISSIndicesBenchmarkFixture, FAISS_IndexPQ_Search)(benchmark::State &state) {
    size_t dim = g_fashion_mnist_data[0].dimension();
    size_t nbits = 8;
    // size_t ncentroids = 256; // Unused in this context, consider removing or using it
    size_t m = 16;

    faiss::IndexFlatL2 quantizer(dim);
    faiss::IndexPQ index(dim, m, nbits);

    // Prepare data
    std::vector<float> data;
    data.reserve(g_fashion_mnist_data.size() * 784);
    for (const auto &point : g_fashion_mnist_data) {
        const std::vector<float> &coords = point.coordinates();
        data.insert(data.end(), coords.begin(), coords.end());
    }

    // Train the index
    index.train(g_fashion_mnist_data.size(), data.data());

    // Add data to the index
    index.add(g_fashion_mnist_data.size(), data.data());

    // Generate query points
    size_t num_queries = 100;
    size_t k = 5;
    std::vector<Point> queries = GenerateRandomQueries(num_queries, 784);
    std::vector<float> query_data;
    query_data.reserve(num_queries * 784);
    for (const auto &query : queries) {
        const std::vector<float> &coords = query.coordinates();
        query_data.insert(query_data.end(), coords.begin(), coords.end());
    }

    for (auto _ : state) {
        std::vector<faiss::idx_t> labels(k * num_queries);
        std::vector<float> distances(k * num_queries);
        index.search(num_queries, query_data.data(), k, distances.data(), labels.data());
        benchmark::DoNotOptimize(labels);
        benchmark::DoNotOptimize(distances);
    }
}

BENCHMARK_REGISTER_F(FAISSIndicesBenchmarkFixture, FAISS_IndexPQ_Search)->Unit(benchmark::kMicrosecond)->Iterations(10);

// Main function for benchmarks
BENCHMARK_MAIN();
