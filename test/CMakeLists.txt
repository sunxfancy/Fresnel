message(STATUS "Linking: ${LIBS}")

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
target_link_libraries(test ${CMAKE_PROJECT_NAME} ${LIBS})
add_executable(test2 ${CMAKE_CURRENT_SOURCE_DIR}/test2.cpp)
target_link_libraries(test2 ${CMAKE_PROJECT_NAME} ${LIBS})