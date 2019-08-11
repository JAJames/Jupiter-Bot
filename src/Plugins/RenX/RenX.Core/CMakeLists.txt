add_plugin(RenX.Core
        RenX.h
        RenX_BanDatabase.cpp
        RenX_BanDatabase.h
        RenX_BuildingInfo.h
        RenX_Core.cpp
        RenX_Core.h
        RenX_ExemptionDatabase.cpp
        RenX_ExemptionDatabase.h
        RenX_Functions.cpp
        RenX_Functions.h
        RenX_GameCommand.cpp
        RenX_GameCommand.h
        RenX_LadderDatabase.cpp
        RenX_LadderDatabase.h
        RenX_Map.cpp
        RenX_Map.h
        RenX_PlayerInfo.h
        RenX_Plugin.cpp
        RenX_Plugin.h
        RenX_Server.cpp
        RenX_Server.h
        RenX_Tags.cpp
        RenX_Tags.h
        RenX_TeamInfo.h)

target_compile_definitions(RenX.Core PRIVATE
        RENX_EXPORTS)