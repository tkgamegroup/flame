file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(SLOC "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(SLOC flame_file)
target_link_libraries(SLOC flame_system)
