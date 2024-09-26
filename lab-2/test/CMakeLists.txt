include(CMakeLists.gtest.txt)

# Recursively find all test source files (*.cpp) in the test directory and its
# subdirectories
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Iterate over each test source file and add it as a test executable
foreach(TEST_SRC ${TEST_SOURCES})
  # Extract the test name by removing the directory path and file extension
  get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)

  add_gtest(${TEST_NAME} ${TEST_SRC})
endforeach()
