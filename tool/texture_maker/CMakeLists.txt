file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(texture_maker "${SOURCE_LIST}" "tool")
target_link_libraries(texture_maker flame_foundation)
