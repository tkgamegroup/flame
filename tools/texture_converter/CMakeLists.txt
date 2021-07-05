file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(texture_converter ${source_files})
set_target_properties(texture_converter PROPERTIES FOLDER "tools")
target_link_libraries(texture_converter flame_graphics)
