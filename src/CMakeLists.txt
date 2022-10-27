
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

##################################################################
## source code configuration
file(GLOB_RECURSE header_files ${CMAKE_SOURCE_DIR}/include/**/*.hpp)
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/deps.cpp
        COMMAND python ${CMAKE_SOURCE_DIR}/tools/gen_deps.py include  ${CMAKE_CURRENT_BINARY_DIR}/deps.cpp
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/tools/gen_deps.py 
        DEPENDS ${header_files} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

function(add_spirv_shader SHADER_STAGE INPUT_FILE OUTPUT_FILE)
add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslc --target-env=vulkan1.2 -fshader-stage=${SHADER_STAGE} ${INPUT_FILE} -o ${OUTPUT_FILE}
        MAIN_DEPENDENCY ${INPUT_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endfunction()

add_spirv_shader(vertex ${CMAKE_SOURCE_DIR}/shader/vert.glsl vert.spv)
add_spirv_shader(fragment ${CMAKE_SOURCE_DIR}/shader/frag.glsl frag.spv)

add_library(${CMAKE_PROJECT_NAME} ${source_files} ${CMAKE_CURRENT_BINARY_DIR}/deps.cpp)
add_custom_target(shaders ALL DEPENDS vert.spv frag.spv)

