add_renx_plugin(RenX.ServerList
        RenX_ServerList.cpp
        RenX_ServerList.h)

target_link_libraries(RenX.ServerList
        HTTPServer)