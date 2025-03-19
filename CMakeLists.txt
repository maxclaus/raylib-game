cmake_minimum_required(VERSION 3.24)
project(game)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_SOURCE_DIR}/src)

# Enable strict compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wstrict-prototypes)

# macOS framework linking
if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()
