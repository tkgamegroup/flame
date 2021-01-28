file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(texture_converter ${SOURCES})
set_target_properties(texture_converter PROPERTIES FOLDER "tools")
target_link_libraries(texture_converter flame_graphics)
