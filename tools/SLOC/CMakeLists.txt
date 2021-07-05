file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(SLOC ${source_files})
set_target_properties(SLOC PROPERTIES FOLDER "tools")
target_link_libraries(SLOC flame_foundation)
