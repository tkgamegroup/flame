file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(graphics_test "${SOURCE_LIST}" "tests")
target_link_libraries(graphics_test flame_graphics)
