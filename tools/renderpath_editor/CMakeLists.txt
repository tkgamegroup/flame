file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(renderpath_editor "${SOURCE_LIST}" "tools")
target_link_libraries(renderpath_editor flame_UI)

