add_subdirectory(lib)

# Executables
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

add_executable(proto_example proto_example.cpp)
target_link_libraries(proto_example PRIVATE ${PROJECT_NAME}_lib)
