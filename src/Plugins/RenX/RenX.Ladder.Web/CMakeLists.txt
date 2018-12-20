add_renx_plugin(RenX.Ladder.Web
        RenX_Ladder_Web.cpp
        RenX_Ladder_Web.h)

target_link_libraries(RenX.Ladder.Web
        HTTPServer)