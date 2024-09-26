include(CMakeLists.benchmark.txt)

# Recursively find all benchmark source files (*.cpp) in the benchmark directory and its subdirectories
file(GLOB_RECURSE BENCH_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Iterate over each benchmark source file and add it as a benchmark executable
foreach(BENCH_SRC ${BENCH_SOURCES})
  # Extract the benchmark name by removing the directory path and file extension
  get_filename_component(BENCH_NAME ${BENCH_SRC} NAME_WE)
  
  # Add the benchmark using the ADD_BENCHMARK function
  ADD_BENCHMARK(${BENCH_NAME} ${BENCH_SRC})
endforeach()
