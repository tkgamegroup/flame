file(GLOB_RECURSE SOURCES "*.c*")
add_executable(graphics_test_canvas ${SOURCES})
set_target_properties(graphics_test_canvas PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_canvas flame_graphics)
