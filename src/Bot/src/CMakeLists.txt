cmake_minimum_required(VERSION 3.0)

# Setup source files
set(SOURCE_FILES
        Console_Command.cpp
        IRC_Bot.cpp
        IRC_Command.cpp
        Main.cpp
        ServerManager.cpp)

# Setup executable build target
add_executable(Bot ${SOURCE_FILES})

# Setup include directories
target_include_directories(Bot PUBLIC ../include)

# Setup additionally needed libs
target_link_libraries(Bot jupiter jessilib)

# Enable exports
set_target_properties(Bot PROPERTIES
        ENABLE_EXPORTS on)

target_compile_definitions(Bot PRIVATE
        JUPITER_BOT_EXPORTS)