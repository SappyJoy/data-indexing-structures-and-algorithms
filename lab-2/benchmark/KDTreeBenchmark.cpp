#include "kdtree/KDTree.hpp"
#include <benchmark/benchmark.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace kdtree;

// std::vector<Point> loadData(const std::string& filename, size_t max_points = 0) {
//     std::vector<Point> points;
//     std::ifstream file(filename);
//     std::string line;
//     size_t count = 0;
//
//     while (std::getline(file, line)) {
//         if (max_points > 0 && count >= max_points) break;
//         std::stringstream ss(line);
//         std::string value;
//         std::vector<double> coords;
//
//         // Parse the first 10 numerical features
//         for (int i = 0; i < 10; ++i) {
//             if (!std::getline(ss, value, ',')) break;
//             coords.push_back(std::stod(value));
//         }
//
//         points.emplace_back(coords);
//         ++count;
//     }
//     return points;
// }
//
// static void BM_KDTreeConstruction(benchmark::State& state) {
//     size_t num_points = state.range(0);
//     auto points = loadData("data/covtype.data", num_points);
//
//     for (auto _ : state) {
//         KDTree tree(10); // 10 dimensions
//         for (const auto& point : points) {
//             tree.insert(point);
//         }
//     }
//
//     state.SetItemsProcessed(state.iterations() * num_points);
// }
//
// BENCHMARK(BM_KDTreeConstruction)->Arg(1000)->Arg(10000)->Arg(100000);
//
// static void BM_KDTreeKNNQuery(benchmark::State& state) {
//     size_t num_points = state.range(0);
//     size_t k = state.range(1);
//     auto points = loadData("data/covtype.data", num_points);
//
//     // Build KD-tree once
//     KDTree tree(10);
//     for (const auto& point : points) {
//         tree.insert(point);
//     }
//
//     // Use a fixed target point (e.g., the first point)
//     Point target = points[0];
//
//     for (auto _ : state) {
//         auto neighbors = tree.kNearestNeighbors(target, k);
//         benchmark::DoNotOptimize(neighbors);
//     }
//
//     state.SetItemsProcessed(state.iterations() * k);
// }
//
// BENCHMARK(BM_KDTreeKNNQuery)->Args({1000, 5})->Args({10000, 5})->Args({100000, 5});
//
// static void BM_KDTreeRangeQuery(benchmark::State& state) {
//     size_t num_points = state.range(0);
//     auto points = loadData("data/covtype.data", num_points);
//
//     // Build KD-tree once
//     KDTree tree(10);
//     for (const auto& point : points) {
//         tree.insert(point);
//     }
//
//     // Define a range (e.g., ±10 units around the target point)
//     Point lower = points[0];
//     Point upper = points[0];
//     for (size_t i = 0; i < lower.dimensions(); ++i) {
//         lower.setCoordinate(i, lower.getCoordinates()[i] - 10);
//         upper.setCoordinate(i, upper.getCoordinates()[i] + 10);
//     }
//
//     for (auto _ : state) {
//         auto results = tree.rangeQuery(lower, upper);
//         benchmark::DoNotOptimize(results);
//     }
//
//     state.SetItemsProcessed(state.iterations());
// }
//
// BENCHMARK(BM_KDTreeRangeQuery)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
