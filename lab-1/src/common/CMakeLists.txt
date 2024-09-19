add_library(${PROJECT_NAME}_common OBJECT Utils.cpp)

set(ALL_OBJECT_FILES
        ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:${PROJECT_NAME}_common>
        PARENT_SCOPE)

target_include_directories(${PROJECT_NAME}_common PUBLIC ${PROJECT_SOURCE_DIR}/include)
