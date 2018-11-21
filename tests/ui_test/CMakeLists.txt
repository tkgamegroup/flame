file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_lib(ui_test "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tests")
target_link_libraries(ui_test flame_ui)

