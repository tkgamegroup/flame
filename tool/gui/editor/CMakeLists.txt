file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(editor "${SOURCE_LIST}" "tools")
target_link_libraries(editor flame_universe)

