file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(graphics_test_draw_image ${SOURCE_LIST})
set_target_properties(graphics_test_draw_image PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_draw_image flame_graphics)