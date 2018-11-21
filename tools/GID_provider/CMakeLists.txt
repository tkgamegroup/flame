file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(GID_provider "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(GID_provider flame_file)
target_link_libraries(GID_provider flame_system)
