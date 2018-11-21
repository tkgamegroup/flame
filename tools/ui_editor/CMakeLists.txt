file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_lib(ui_editor "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(ui_editor flame_ui)

