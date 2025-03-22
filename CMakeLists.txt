cmake_minimum_required(VERSION 3.31)
project(game)

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
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.

  # Configure assets for web
  set(WEB_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE STRING "dir for assets")
  string(APPEND WEB_ASSETS_DIR "@assets") # It makes local resources path be mapped to /assets virtual path
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--preload-file ${WEB_ASSETS_DIR}")
endif ()

# Enable strict compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

# macOS framework linking
if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
