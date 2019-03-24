file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(SLOC "${SOURCE_LIST}" "tool")
target_link_libraries(SLOC flame_foundation)
