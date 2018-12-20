macro(add_plugin target)
    add_library(${target} SHARED ${ARGN})
    target_link_libraries(${target} Bot)
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