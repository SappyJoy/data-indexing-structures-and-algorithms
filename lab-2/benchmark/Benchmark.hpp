#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include "Index.hpp"
#include "Point.hpp"
#include <vector>

namespace benchmark {

using namespace kdtree;

class Benchmark {
public:
    Benchmark(std::shared_ptr<index::Index> index,
              const std::vector<Point>& dataset,
              const std::vector<Point>& queries,
              size_t k);

    void run();

private:
    std::shared_ptr<index::Index> index_;
    std::vector<Point> dataset_;
    std::vector<Point> queries_;
    size_t k_;

    void benchmark_build();
    void benchmark_queries();
};

}  // namespace benchmark

#endif
