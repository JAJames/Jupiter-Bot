cmake_minimum_required(VERSION 3.0)
project(jupiter_bot)

set(CMAKE_CXX_STANDARD 20)
option(INSTALL_JUPITER_BOT "Enables the INSTALL target for Jupiter Bot" ON)

# Try to set JUPITER_VERSION_SHORT if it isn't already
if (NOT DEFINED JUPITER_VERSION_SHORT)
    # Gets the version string portion a release/ branch name if on a release/ branch,
    # gets "{branch_name}-{short_commit_hash}" otherwise
    find_package(Git)
    if (GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
            OUTPUT_VARIABLE JUPITER_VERSION_SHORT
            RESULT_VARIABLE git_result
        )
        string(STRIP "${JUPITER_VERSION_SHORT}" JUPITER_VERSION_SHORT)

        # Append sha1 if not release/
        if (git_result EQUAL 0
            AND NOT JUPITER_VERSION_SHORT MATCHES "^release/.*")
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                OUTPUT_VARIABLE GIT_REVISION
                RESULT_VARIABLE git_result
            )
            string(STRIP "${GIT_REVISION}" GIT_REVISION)

            if (git_result EQUAL 0)
                set(JUPITER_VERSION_SHORT "${JUPITER_VERSION_SHORT}-${GIT_REVISION}")
            endif()
        elseif(JUPITER_VERSION_SHORT MATCHES "^release/*")
            # set JUPITER_VERSION_SHORT to end of release/
            string(REGEX REPLACE "^release/" "" JUPITER_VERSION_SHORT "${JUPITER_VERSION_SHORT}")
        endif()
    endif()
endif()

message(STATUS "JUPITER_VERSION_SHORT: >${JUPITER_VERSION_SHORT}<")
if (DEFINED JUPITER_VERSION_SHORT)
    add_compile_definitions(JUPITER_VERSION_SHORT="${JUPITER_VERSION_SHORT}")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
include(build/CMakeLists.txt)

# Setup source files
add_subdirectory(src)

########################################
# PackagedBuild & FullBotBuild targets #
########################################

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

##########################
# INSTALL rules & target #
##########################

if (INSTALL_JUPITER_BOT)
    message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}") # Prefixed onto all of the below by CMake before use
    message(STATUS "CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_BINDIR}") # bin; Used for executable files (jupiter_bot) & .dll libraries
    message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}") # lib; Used for .so libraries
    message(STATUS "CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}") # include; Not used yet
    message(STATUS "CMAKE_INSTALL_SYSCONFDIR: ${CMAKE_INSTALL_SYSCONFDIR}") # etc; Used for baselines (configs)
    message(STATUS "CMAKE_INSTALL_LOCALSTATEDIR: ${CMAKE_INSTALL_LOCALSTATEDIR}") # var; Not used (yet?)
    message(STATUS "CMAKE_INSTALL_DATADIR: ${CMAKE_INSTALL_DATADIR}") # blank; Not used (yet?)

    set(JUPITER_BOT_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
    set(JUPITER_BOT_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    set(JUPITER_BOT_INSTALL_SYSCONFDIR "${CMAKE_INSTALL_SYSCONFDIR}")

    # Jupiter Bot program binary
    set(JUPITER_BOT_INSTALL_EXE "jupiter_bot${CMAKE_EXECUTABLE_SUFFIX}")
    install(PROGRAMS $<TARGET_FILE:Bot>
            DESTINATION ${JUPITER_BOT_INSTALL_BINDIR}
            RENAME ${JUPITER_BOT_INSTALL_EXE})

    # Shared library dependencies
    install(TARGETS jupiter jessilib
            RUNTIME DESTINATION ${JUPITER_BOT_INSTALL_BINDIR}
            LIBRARY DESTINATION ${JUPITER_BOT_INSTALL_LIBDIR})

    # Plugins
    install(TARGETS ${JUPITER_PLUGINS}
            RUNTIME DESTINATION "${JUPITER_BOT_INSTALL_BINDIR}/Plugins"
            LIBRARY DESTINATION "${JUPITER_BOT_INSTALL_LIBDIR}/Plugins")

    # Copy baselines for modification
    set(TMP_INSTALL_FILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install_tmp")
    set(BASELINE_FILES_INSTALL "${TMP_INSTALL_FILE_DIR}/baseline")
    install(CODE
            "execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory \"${CMAKE_CURRENT_SOURCE_DIR}/baseline\" \"${BASELINE_FILES_INSTALL}/\")")

    # Configure ConfigsDirectory & PluginsDirectory
    set(INSTALL_CONFIG_PATH "${BASELINE_FILES_INSTALL}/Config.ini")
    install(CODE
            "file(APPEND \"${INSTALL_CONFIG_PATH}\" \"ConfigsDirectory=${CMAKE_INSTALL_PREFIX}/${JUPITER_BOT_INSTALL_SYSCONFDIR}/jupiter_bot/Configs\n\")")
    if (WIN32)
        install(CODE
                "file(APPEND \"${INSTALL_CONFIG_PATH}\" \"PluginsDirectory=${CMAKE_INSTALL_PREFIX}/${JUPITER_BOT_INSTALL_BINDIR}/Plugins\n\")")
    else()
        install(CODE
                "file(APPEND \"${INSTALL_CONFIG_PATH}\" \"PluginsDirectory=${CMAKE_INSTALL_PREFIX}/${JUPITER_BOT_INSTALL_LIBDIR}/Plugins\n\")")
    endif()

    # Generate run script to pass in Config.ini
    set(HELPER_SCRIPT_NAME "jupiter_bot.sh")
    if (WIN32)
        install(CODE
                "file(WRITE \"${TMP_INSTALL_FILE_DIR}/jupiter_bot.bat\" \"jupiter_bot.exe --config=${CMAKE_INSTALL_PREFIX}/${JUPITER_BOT_INSTALL_SYSCONFDIR}/jupiter_bot/Config.ini\")")
        install(PROGRAMS ${TMP_INSTALL_FILE_DIR}/jupiter_bot.bat
                DESTINATION ${JUPITER_BOT_INSTALL_BINDIR})
    else()
        install(CODE
                "file(WRITE \"${TMP_INSTALL_FILE_DIR}/jupiter_bot.sh\" \"jupiter_bot --config=${CMAKE_INSTALL_PREFIX}/${JUPITER_BOT_INSTALL_SYSCONFDIR}/jupiter_bot/Config.ini\")")
        install(PROGRAMS ${TMP_INSTALL_FILE_DIR}/jupiter_bot.sh
                DESTINATION ${JUPITER_BOT_INSTALL_BINDIR})
    endif()

    # Copy configured baseline files
    install(DIRECTORY ${BASELINE_FILES_INSTALL}/
            DESTINATION "${JUPITER_BOT_INSTALL_SYSCONFDIR}/jupiter_bot")
endif()
