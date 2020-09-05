file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(model_converter ${SOURCE_LIST})
set_target_properties(model_converter PROPERTIES FOLDER "tools")
target_link_libraries(model_converter flame_graphics)
