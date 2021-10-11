file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(create_template ${source_files})
set_target_properties(create_template PROPERTIES FOLDER "tools")
target_link_libraries(create_template flame_foundation)
