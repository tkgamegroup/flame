file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(GID_provider "${SOURCE_LIST}" "tools")
target_link_libraries(GID_provider flame_foundation)
