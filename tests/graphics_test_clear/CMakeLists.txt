file(GLOB_RECURSE SOURCES "*.c*")
add_executable(graphics_test_clear ${SOURCES})
set_target_properties(graphics_test_clear PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_clear flame_graphics)