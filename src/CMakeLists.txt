# Setup output paths for jupiter and Bot
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_SHARED_LIBRARY_PREFIX)

# Add primary components (Jupiter, Bot, jessilib)
add_subdirectory(Jupiter)
add_subdirectory(jessilib)
add_subdirectory(Bot)

# Setup output paths for plugins
add_subdirectory(Plugins)

# Propagate JUPITER_PLUGINS upwards
set(JUPITER_PLUGINS "${JUPITER_PLUGINS}" PARENT_SCOPE)