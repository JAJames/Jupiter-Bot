add_plugin(HTTPServer
        HTTPServer.cpp
        HTTPServer.h)

target_compile_definitions(HTTPServer PRIVATE
        HTTPSERVER_EXPORTS)

target_include_directories(HTTPServer PUBLIC .)