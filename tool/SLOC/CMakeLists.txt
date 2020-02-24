file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(SLOC ${SOURCE_LIST})
set_target_properties(SLOC PROPERTIES FOLDER "tool")
target_link_libraries(SLOC flame_foundation)
