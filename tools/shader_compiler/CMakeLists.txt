file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(shader_compiler ${SOURCES})
set_target_properties(shader_compiler PROPERTIES FOLDER "tools")
target_link_libraries(shader_compiler flame_graphics)
