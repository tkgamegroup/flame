file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(when-changed "${SOURCE_LIST}" "tool")
target_link_libraries(when-changed flame_foundation)

