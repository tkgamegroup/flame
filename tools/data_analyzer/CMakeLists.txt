file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(data_analyzer ${source_files})
set_target_properties(data_analyzer PROPERTIES FOLDER "tools")
target_link_libraries(data_analyzer flame_foundation)
