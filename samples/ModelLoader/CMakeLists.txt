# project(ModelLoader)

file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

add_executable(ModelLoader ${source_files})
target_link_libraries(ModelLoader ${CMAKE_PROJECT_NAME} ${LIBS})
