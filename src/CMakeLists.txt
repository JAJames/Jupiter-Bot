# Setup output paths for jupiter and Bot
# TODO: these paths probably aren't needed anymore, per PackagedLib target, though can still be useful when running from
# repo dir. Not sure if anybody is relying on that functionality
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_SHARED_LIBRARY_PREFIX)

# We need Jupiter and jessilib to be shared libs, and there aren't separate targets for shared/static variants
set(BUILD_SHARED_LIBS ON)

# Add primary components (Jupiter, Bot)
add_subdirectory(Jupiter)
add_subdirectory(Bot)

# Setup output paths for plugins
add_subdirectory(Plugins)

# Propagate JUPITER_PLUGINS upwards
set(JUPITER_PLUGINS "${JUPITER_PLUGINS}" PARENT_SCOPE)