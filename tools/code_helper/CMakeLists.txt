file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(code_helper ${source_files})
set_target_properties(code_helper PROPERTIES FOLDER "tools")
target_link_libraries(code_helper flame_foundation)
