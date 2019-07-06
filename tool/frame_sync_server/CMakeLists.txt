file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(frame_sync_server "${SOURCE_LIST}" "tool")
target_link_libraries(frame_sync_server flame_network)
