file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(frame_sync_server ${SOURCES})
set_target_properties(frame_sync_server PROPERTIES FOLDER "tools")
target_link_libraries(frame_sync_server flame_network)
