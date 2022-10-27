# find the required packages
option(AUTO_LOCATE_VULKAN "AUTO_LOCATE_VULKAN" ON)
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan in ${Vulkan_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")


# first create relevant static libraries requried for other projects
file(GLOB_RECURSE IMGUI_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/*.cpp)
add_library(imgui ${IMGUI_SRC})

add_library(stb_image "third-party/src/stb_image.cpp")
set(LIBS ${LIBS} stb_image)

file(GLOB_RECURSE SOIL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third-party/SOIL/*.c)
add_library(SOIL ${SOIL_SRC})
set(LIBS ${LIBS} SOIL)

add_library(glad "third-party/src/glad.c")
set(LIBS ${LIBS} glad)

add_library(pugixml "third-party/src/pugixml.cpp")
set(LIBS ${LIBS} pugixml)

add_library(noiseutils "third-party/src/noiseutils.cpp")
set(LIBS ${LIBS} noise noiseutils)

add_library(VkBootstrap "third-party/src/VkBootstrap.cpp")
set(LIBS ${LIBS} VkBootstrap)

# For PhysX
## include path configuration
include_directories(third-party/PhysX/include third-party/PhysX/pxshared/include)
## library configuration
if(CMAKE_BUILD_TYPE MATCHES Release)
  link_directories(${CMAKE_SOURCE_DIR}/third-party/PhysX/lib)
else()
  link_directories(${CMAKE_SOURCE_DIR}/third-party/PhysX/lib_debug)
endif()
set(LIBS ${LIBS} PhysXFoundation_64 PhysXCommon_64 PhysXCooking_64 PhysX_64)
set(LIBS ${LIBS} PhysXExtensions_static_64)

set (TBB_ROOT_DIR ${CMAKE_SOURCE_DIR}/third-party/tbb)
find_package(TBB REQUIRED)
message(STATUS "Found TBB in ${TBB_INCLUDE_DIRS}")

# for freetype
include_directories(third-party/include/misc/freetype)


# copy dlls
file(GLOB DLLS "${CMAKE_SOURCE_DIR}/third-party/dlls/*.dll")
file(COPY ${DLLS} DESTINATION ${CMAKE_BINARY_DIR})

set(LIBS ${LIBS} ${ASSIMP_LIBRARY} imgui freetype SDL2 ${Vulkan_LIBRARY} ${TBB_LIBRARY} glfw3)


## path configuration
include_directories(third-party/include ${Vulkan_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS})
link_directories(${CMAKE_SOURCE_DIR}/third-party/lib )


##################################################################
## add platform specific libraries
if(WIN32) 
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_definitions(
    _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH )
  set(LIBS ${LIBS} opengl32 )
  # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(X11 REQUIRED)
  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(LIBS ${LIBS} X11 Xrandr Xinerama Xi Xxf86vm Xcursor dl pthread )
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)

  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY})

  SET(APPLE_LIBS ${APPLE_LIBS} ${COCOA_LIBRARY} ${IOKit_LIBRARY})
  set(LIBS ${LIBS} ${APPLE_LIBS})
endif()
