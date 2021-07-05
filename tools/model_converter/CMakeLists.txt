file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(model_converter ${source_files})
set_target_properties(model_converter PROPERTIES FOLDER "tools")
target_link_libraries(model_converter flame_graphics)
