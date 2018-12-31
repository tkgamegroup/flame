file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(SLOC "${SOURCE_LIST}" "tools")
target_link_libraries(SLOC flame_foundation)
