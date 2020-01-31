file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(compiler "${SOURCE_LIST}" "tool")
target_link_libraries(compiler flame_foundation)

