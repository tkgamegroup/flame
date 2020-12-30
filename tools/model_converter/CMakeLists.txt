file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(model_converter ${SOURCES})
set_target_properties(model_converter PROPERTIES FOLDER "tools")
target_link_libraries(model_converter flame_graphics)
