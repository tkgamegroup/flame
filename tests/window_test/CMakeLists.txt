file(GLOB_RECURSE source_files "*.c*")
add_executable(window_test ${source_files})
set_target_properties(window_test PROPERTIES FOLDER "tests")
target_link_libraries(window_test flame_foundation)
