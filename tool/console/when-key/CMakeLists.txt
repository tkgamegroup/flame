file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(when-key "${SOURCE_LIST}" "tool")
target_link_libraries(when-key flame_foundation)

