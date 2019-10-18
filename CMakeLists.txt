cmake_minimum_required(VERSION 3.0)
project(jupiter_bot)

set(CMAKE_CXX_STANDARD 11)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
include(build/CMakeLists.txt)

# Setup source files
add_subdirectory(src)
