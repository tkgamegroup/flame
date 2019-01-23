file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(blueprint_editor "${SOURCE_LIST}" "tools")
target_link_libraries(blueprint_editor flame_foundation)

