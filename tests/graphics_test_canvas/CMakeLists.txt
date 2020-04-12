file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(graphics_test_canvas ${SOURCE_LIST})
set_target_properties(graphics_test_canvas PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_canvas flame_graphics)
