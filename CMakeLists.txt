cmake_minimum_required(VERSION 3.0)
project(jupiter_bot)

set(CMAKE_CXX_STANDARD 20)

if (DEFINED JUPITER_VERSION_SHORT)
    add_compile_definitions(JUPITER_VERSION_SHORT="${JUPITER_VERSION_SHORT}")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
include(build/CMakeLists.txt)

# Setup source files
add_subdirectory(src)

# Add PackagedBuild target to package a release build, and FullBotBuild to just build all plugins
add_custom_target(FullBotBuild)
add_dependencies(FullBotBuild jupiter Bot)

if(NOT DEFINED PACKAGED_BUILD_PATH)
    set(PACKAGED_BUILD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()
message(STATUS "PackagedBuild Path: ${PACKAGED_BUILD_PATH}")

set(jessilib_target_file)
get_target_property(jessilib_target_type jessilib TYPE)
if (jessilib_target_type STREQUAL "SHARED_LIBRARY")
    set(jessilib_target_file "$<TARGET_FILE:jessilib>")
endif()

set(jupiter_target_file)
get_target_property(jupiter_target_type jupiter TYPE)
if (jupiter_target_type STREQUAL "SHARED_LIBRARY")
    set(jupiter_target_file "$<TARGET_FILE:jupiter>")
endif()

add_custom_target(PackagedBuild)
add_dependencies(PackagedBuild FullBotBuild)
add_custom_command(TARGET PackagedBuild
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${PACKAGED_BUILD_PATH})
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGED_BUILD_PATH}/Plugins/)
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${jupiter_target_file} ${jessilib_target_file} $<TARGET_FILE:Bot> ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${PACKAGED_BUILD_PATH}/)
add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/baseline ${PACKAGED_BUILD_PATH}/)

foreach(plugin ${JUPITER_PLUGINS})
    add_dependencies(FullBotBuild ${plugin})
    add_custom_command(TARGET PackagedBuild
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${plugin}> ${PACKAGED_BUILD_PATH}/Plugins/)
endforeach()

# Wrap it all into a .zip / .tar.gz
set(TAR_FORMAT)
set(TAR_EXTENSION ".tar.gz")
set(TAR_CREATE_FLAGS "cfz")
if (WIN32)
    set(TAR_FORMAT "--format=zip")
    set(TAR_EXTENSION ".zip")
    set(TAR_CREATE_FLAGS "cf")
endif()

if (DEFINED JUPITER_VERSION_SHORT)
    set(TAR_EXTENSION ".${JUPITER_VERSION_SHORT}${TAR_EXTENSION}")
endif()

set(TAR_FILENAME "Jupiter Bot Binaries${TAR_EXTENSION}")

add_custom_command(TARGET PackagedBuild
        POST_BUILD
        WORKING_DIRECTORY "${PACKAGED_BUILD_PATH}/"
        COMMAND ${CMAKE_COMMAND} -E chdir "${PACKAGED_BUILD_PATH}/" tar ${TAR_CREATE_FLAGS} ../${TAR_FILENAME} ${TAR_FORMAT} -- *)

add_custom_command(TARGET PackagedBuild
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "${PACKAGED_BUILD_PATH}/../${TAR_FILENAME}" "${PACKAGED_BUILD_PATH}/${TAR_FILENAME}")
