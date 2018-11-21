file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_lib(ui_test "${SOURCE_LIST}" "tests")
target_link_libraries(ui_test flame_ui)

