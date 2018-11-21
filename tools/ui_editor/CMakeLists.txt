file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_lib(ui_editor "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(ui_editor flame_ui)

