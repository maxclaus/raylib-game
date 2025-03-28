cmake_minimum_required(VERSION 3.31)
project(game)


# we default to Release build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
include(FetchContent)

set(RAYLIB_VERSION 5.5)
FetchContent_Declare(
  raylib
  URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)

set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(raylib)

# Our project
set(SOURCE_FILES main.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

if (${PLATFORM} STREQUAL "Web")
  message(STATUS "Building for web with emscripten")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ./_deps/raylib-src/src/minshell.html")
  endif()

  # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

  # Configure assets for web
  set(WEB_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE STRING "dir for assets")
  string(APPEND WEB_ASSETS_DIR "@assets") # It makes local resources path be mapped to /assets virtual path
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--preload-file ${WEB_ASSETS_DIR}")
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building in debug mode")
  target_compile_options(${PROJECT_NAME} PRIVATE -g -DDEBUG -Wall -Wextra -Wpedantic -Werror)
else()
  message(STATUS "Building in release mode")
  target_compile_options(${PROJECT_NAME} PRIVATE -O3 -Wall -Wextra -Wpedantic -Werror)
endif()

# macOS framework linking
if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
