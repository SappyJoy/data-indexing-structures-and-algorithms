# Project library
add_library(${PROJECT_NAME}_lib STATIC ${ALL_OBJECT_FILES} Point.cpp Node.cpp BoundingBox.cpp KDTree.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC fmt::fmt protobuf_generated)
target_include_directories(
  ${PROJECT_NAME}_lib PUBLIC ${PROJECT_SOURCE_DIR}/include
                             ${CMAKE_BINARY_DIR}/protoc)
