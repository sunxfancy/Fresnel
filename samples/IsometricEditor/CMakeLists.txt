# project(IsometricEditor)

file(GLOB_RECURSE ie_source_files ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

message(STATUS "SubLinking: ${LIBS}")
add_executable(IsometricEditor ${ie_source_files})
target_link_libraries(IsometricEditor ${CMAKE_PROJECT_NAME} ${LIBS})
