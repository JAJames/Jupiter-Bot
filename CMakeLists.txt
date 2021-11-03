cmake_minimum_required(VERSION 3.0)
project(jupiter_bot)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
include(build/CMakeLists.txt)

# Setup source files
add_subdirectory(src)

# Add PackagedBuild target to package a release build
add_custom_target(PackagedBuild)
add_dependencies(PackagedBuild jupiter Bot)

if(NOT DEFINED ${PACKAGED_BUILD_PATH})
    set(PACKAGED_BUILD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

add_custom_command(TARGET PackagedBuild
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${PACKAGED_BUILD_PATH})
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGED_BUILD_PATH}/Plugins/)
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:jupiter> $<TARGET_FILE:Bot> ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${PACKAGED_BUILD_PATH}/)
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/baseline ${PACKAGED_BUILD_PATH}/)

foreach(plugin ${JUPITER_PLUGINS})
    add_dependencies(PackagedBuild ${plugin})
    add_custom_command(TARGET PackagedBuild
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${plugin}> ${PACKAGED_BUILD_PATH}/Plugins/)
endforeach()
