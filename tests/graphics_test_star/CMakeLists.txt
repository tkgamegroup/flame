file(GLOB_RECURSE source_files "*.c*")
add_executable(graphics_test_star ${source_files})
set_target_properties(graphics_test_star PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_star flame_graphics)
