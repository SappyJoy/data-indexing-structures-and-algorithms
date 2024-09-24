find_package(Protobuf REQUIRED CONFIG)

file(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)

# Generate protobuf files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Create a single library with all protobuf sources
add_library(protobuf_generated STATIC ${PROTO_SRCS} ${PROTO_HDRS})

# Set include directories for protobuf
target_include_directories(protobuf_generated PUBLIC ${Protobuf_INCLUDE_DIRS})

# Link protobuf library to the generated files
target_link_libraries(protobuf_generated PUBLIC protobuf::libprotobuf)
