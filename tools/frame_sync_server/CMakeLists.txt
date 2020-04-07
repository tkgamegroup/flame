file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(frame_sync_server ${SOURCE_LIST})
set_target_properties(frame_sync_server PROPERTIES FOLDER "tools")
target_link_libraries(frame_sync_server flame_network)
