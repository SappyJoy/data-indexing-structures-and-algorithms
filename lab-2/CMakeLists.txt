cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(
  kdtree
  LANGUAGES CXX
  VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message("-- Using Clang compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message("-- Using GCC compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  message("-- Using Intel compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message("-- Using MSVC compiler")
endif()

# Better set this in the command line
# set(CMAKE_BUILD_TYPE
#     "Release"
#     CACHE STRING "" FORCE)

# Faiss General options
set(FAISS_ENABLE_GPU
    OFF
    CACHE BOOL "" FORCE)
set(FAISS_ENABLE_PYTHON
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_TESTING
    OFF
    CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE) # Or ON if you prefer shared libraries
set(FAISS_ENABLE_C_API
    OFF
    CACHE BOOL "" FORCE)
set(FAISS_OPT_LEVEL
    "generic"
    CACHE STRING "" FORCE) # Options: "generic", "avx2", "avx512"
set(FAISS_USE_OPENMP
    ON
    CACHE BOOL "" FORCE)

# BLAS-related options
set(BLA_VENDOR
    "OpenBLAS"
    CACHE STRING "" FORCE) # Change to "Intel10_64_dyn" if using MKL

# Optimization options
set(FAISS_USE_LTO
    OFF
    CACHE BOOL "" FORCE) # Enable if you want Link-Time Optimization

set(BLAS_LIBRARIES ${CONAN_LIBS_OPENBLAS})
set(BLAS_INCLUDE_DIR ${CONAN_INCLUDE_DIRS_OPENBLAS})
set(BLAS_FOUND TRUE)
set(FAISS_ENABLE_CBLAS OFF CACHE BOOL "" FORCE)

# Code completion. Generates compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenMP REQUIRED)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR
    ${PROJECT_SOURCE_DIR}/third_party
    CACHE PATH "Missing description." FORCE)

FetchContent_Declare(
  faiss
  GIT_REPOSITORY https://github.com/facebookresearch/faiss.git
  GIT_TAG v1.8.0)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2)

# It's better to move to 'test/'
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  CMAKE_CACHE_ARGS "${googletest_cmake_args}")

# Download benchmark lib It's better to move to 'benchmark/'
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.0)

FetchContent_MakeAvailable(faiss fmt googletest googlebenchmark)

# CCache
find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -Wall -Werror -g")
set(CMAKE_CXX_FLAGS_SANITIZED
    "-std=c++17 -Wall -Werror -g -fsanitize=address,undefined,leak")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -Wall -Werror -g -O3")
mark_as_advanced(CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_SANITIZED
                 CMAKE_CXX_FLAGS_RELEASE)

add_subdirectory(protoc)

add_subdirectory(src)

# Testing
enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
