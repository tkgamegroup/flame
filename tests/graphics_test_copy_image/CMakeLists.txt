file(GLOB_RECURSE SOURCES "*.c*")
add_executable(graphics_test_copy_image ${SOURCES})
set_target_properties(graphics_test_copy_image PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_copy_image flame_graphics)