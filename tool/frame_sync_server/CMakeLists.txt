file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(frame_sync_server ${SOURCE_LIST})
set_target_properties(frame_sync_server PROPERTIES FOLDER "tool")
target_link_libraries(frame_sync_server flame_network)
