file(GLOB_RECURSE SOURCES "*.c*")
add_executable(graphics_test_rain ${SOURCES})
set_target_properties(graphics_test_rain PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_rain flame_graphics)
