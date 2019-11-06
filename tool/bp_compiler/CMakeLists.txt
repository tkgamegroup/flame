file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(bp_compiler "${SOURCE_LIST}" "tool")
target_link_libraries(bp_compiler flame_foundation)
