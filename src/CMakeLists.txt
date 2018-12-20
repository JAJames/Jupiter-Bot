# Setup output paths for jupiter and Bot
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_SHARED_LIBRARY_PREFIX)

# Add jupiter and Bot
add_subdirectory(jupiter)
add_subdirectory(Bot)

# Setup output paths for plugins
add_subdirectory(Plugins)