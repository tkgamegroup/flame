file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(primitive_debugger ${source_files})
set_target_properties(primitive_debugger PROPERTIES FOLDER "tools")
target_link_libraries(primitive_debugger flame_graphics)
