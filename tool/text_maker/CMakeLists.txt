file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(text_maker "${SOURCE_LIST}" "tool")
target_link_libraries(text_maker flame_foundation)
