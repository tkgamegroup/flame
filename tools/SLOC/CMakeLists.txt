file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(SLOC ${SOURCES})
set_target_properties(SLOC PROPERTIES FOLDER "tools")
target_link_libraries(SLOC flame_foundation)
