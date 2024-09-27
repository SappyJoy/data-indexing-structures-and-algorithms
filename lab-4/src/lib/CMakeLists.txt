# Project library
add_library(${PROJECT_NAME}_lib STATIC)

file(GLOB_RECURSE LIB_SRC_FILES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(${PROJECT_NAME}_lib PRIVATE ${LIB_SRC_FILES})

target_link_libraries(${PROJECT_NAME}_lib PUBLIC fmt::fmt)
target_include_directories(${PROJECT_NAME}_lib
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
