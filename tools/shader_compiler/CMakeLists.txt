file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(shader_compiler ${source_files})
set_target_properties(shader_compiler PROPERTIES FOLDER "tools")
target_link_libraries(shader_compiler flame_graphics)
