generate_config()
file(GLOB_RECURSE source_files "*.c*")
add_executable(bp_editor ${source_files})
set_target_properties(bp_editor PROPERTIES FOLDER "tools")
target_link_libraries(bp_editor flame_universe)
