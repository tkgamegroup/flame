file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_lib(ui_editor "${SOURCE_LIST}" "tools")
target_link_libraries(ui_editor flame_ui)

