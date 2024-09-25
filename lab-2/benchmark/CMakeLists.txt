include(CMakeLists.benchmark.txt)

add_benchmark(KDTreeBenchmark)
# target_link_libraries(KDTreeBenchmark PRIVATE faiss OpenMP::OpenMP_CXX)
