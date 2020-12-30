file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(texture_maker ${SOURCES})
set_target_properties(texture_maker PROPERTIES FOLDER "tools")
target_link_libraries(texture_maker flame_foundation)
