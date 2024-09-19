add_library(${PROJECT_NAME}_lib STATIC ${ALL_OBJECT_FILES} Bucket.cpp
                                       ExtensibleHashing.cpp)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC fmt::fmt)
target_include_directories(${PROJECT_NAME}_lib
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
