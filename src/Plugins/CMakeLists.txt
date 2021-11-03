# Setup output paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Plugins)

macro(add_plugin target)
    add_library(${target} SHARED ${ARGN})
    target_link_libraries(${target} Bot)
    set(JUPITER_PLUGINS "${JUPITER_PLUGINS}" "${target}" PARENT_SCOPE)
endmacro(add_plugin)

# Add plugins
add_subdirectory(ChannelRelay)
add_subdirectory(CoreCommands)
add_subdirectory(ExtraCommands)
add_subdirectory(FunCommands)
add_subdirectory(HTTPServer)
add_subdirectory(IRC.Core)
add_subdirectory(PluginManager)
add_subdirectory(SetJoin)

# Add RenX plugins
add_subdirectory(RenX)

# Propagate JUPITER_PLUGINS upwards
set(JUPITER_PLUGINS "${JUPITER_PLUGINS}" PARENT_SCOPE)
