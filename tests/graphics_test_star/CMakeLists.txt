file(GLOB_RECURSE SOURCES "*.c*")
add_executable(graphics_test_star ${SOURCES})
set_target_properties(graphics_test_star PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_star flame_graphics)
