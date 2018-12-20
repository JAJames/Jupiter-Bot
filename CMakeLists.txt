cmake_minimum_required(VERSION 3.0)
project(jupiter_bot)

set(CMAKE_CXX_STANDARD 17)

include(build/CMakeLists.txt)

# Setup source files
add_subdirectory(src)
