cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(ehash
        LANGUAGES CXX
        VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        MESSAGE("-- Using Clang compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        MESSAGE("-- Using GCC compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        MESSAGE("-- Using Intel compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        MESSAGE("-- Using MSVC compiler")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -Wall -Werror -g")
set(CMAKE_CXX_FLAGS_SANITIZED "-std=c++17 -Wall -Werror -g -fsanitize=address,undefined,leak")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -Wall -Werror -g -O3")
mark_as_advanced(
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_SANITIZED
        CMAKE_CXX_FLAGS_RELEASE
)

# Code completion. Generates compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/third_party CACHE PATH "Missing description." FORCE)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.1.1
)


# It's better to move to 'test/'
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
  CMAKE_CACHE_ARGS "${googletest_cmake_args}"
)

# Download benchmark lib
# It's better to move to 'benchmark/'
FetchContent_Declare(googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)

FetchContent_MakeAvailable(
    fmt
    googletest
    googlebenchmark
)

# CCache
find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

add_subdirectory(src)

# Testing
enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)
