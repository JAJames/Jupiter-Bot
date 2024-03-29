# RenX.Core
add_subdirectory(RenX.Core)

# TODOJJ: Move headers into an include dir
target_include_directories(RenX.Core PUBLIC
        RenX.Core)

macro(add_renx_plugin target)
    add_plugin(${target} ${ARGN})
    target_link_libraries(${target} RenX.Core)
endmacro(add_renx_plugin)

# Add plugins
add_subdirectory(RenX.AlwaysRecord)
add_subdirectory(RenX.Announcements)
add_subdirectory(RenX.Commands)
add_subdirectory(RenX.ChatLogging)
add_subdirectory(RenX.CommandLogging)
add_subdirectory(RenX.ExcessiveHeadshots)
add_subdirectory(RenX.ExtraLogging)
add_subdirectory(RenX.Greetings)
add_subdirectory(RenX.HybridUUID)
add_subdirectory(RenX.IRCJoin)
add_subdirectory(RenX.KickDupes)
add_subdirectory(RenX.Ladder)
add_subdirectory(RenX.Ladder.All-Time)
add_subdirectory(RenX.Ladder.Daily)
add_subdirectory(RenX.Ladder.Monthly)
add_subdirectory(RenX.Ladder.Web)
add_subdirectory(RenX.Ladder.Weekly)
add_subdirectory(RenX.Ladder.Yearly)
add_subdirectory(RenX.Listen)
add_subdirectory(RenX.Logging)
add_subdirectory(RenX.Medals)
add_subdirectory(RenX.MinPlayers)
add_subdirectory(RenX.ModSystem)
add_subdirectory(RenX.NicknameUUID)
add_subdirectory(RenX.Plugin.Template)
add_subdirectory(RenX.Relay)
add_subdirectory(RenX.ServerList)
add_subdirectory(RenX.SetJoin)
add_subdirectory(RenX.Warn)

# Propagate JUPITER_PLUGINS upwards
set(JUPITER_PLUGINS "${JUPITER_PLUGINS}" PARENT_SCOPE)
