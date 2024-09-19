add_executable(print print.cpp)
target_link_libraries(print benchmark::benchmark ${PROJECT_NAME}_lib)
target_include_directories(print PUBLIC ${PROJECT_SOURCE_DIR}/include)
