file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(packet_extractor ${source_files})
set_target_properties(packet_extractor PROPERTIES FOLDER "tools")
target_link_libraries(packet_extractor flame_foundation)
