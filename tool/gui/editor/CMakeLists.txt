file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(editor_gui "${SOURCE_LIST}" "tool")
target_link_libraries(editor_gui flame_universe)

