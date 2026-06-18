# GLOBAL PART
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(calrc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(${CMAKE_SOURCE_DIR}/include)

# COMPILATION FLAGS
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -Werror -DDEBUG")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

set(sanitizer_flags
    "-fsanitize=undefined"
    "-fsanitize=address"
    "-fsanitize=leak"
)

# TARGET
find_package(OpenMP REQUIRED)

# Header-only fmt library bundled under include/fmt/
add_library(fmt_header_only INTERFACE)
target_include_directories(fmt_header_only INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(fmt_header_only INTERFACE FMT_HEADER_ONLY=1)

add_library(grid_obj OBJECT src/grid.cpp)
target_include_directories(grid_obj PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(grid_obj PRIVATE OpenMP::OpenMP_CXX)

add_executable(calrc src/main.cpp src/args_parser.cpp $<TARGET_OBJECTS:grid_obj>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(calrc PRIVATE ${sanitizer_flags})
    target_link_options(calrc PRIVATE ${sanitizer_flags})
endif()

target_link_libraries(calrc fmt_header_only hdf5 OpenMP::OpenMP_CXX)

# Python module
add_library(pymodule SHARED src/py.cpp $<TARGET_OBJECTS:grid_obj>)
target_link_libraries(pymodule OpenMP::OpenMP_CXX)

# Unit tests
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()
    add_subdirectory(validation)
endif()