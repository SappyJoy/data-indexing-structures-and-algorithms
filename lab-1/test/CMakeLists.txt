include(CMakeLists.gtest.txt)
add_gtest(ExtensibleHashingTest)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test/test_buckets")
