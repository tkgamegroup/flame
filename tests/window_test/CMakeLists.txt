file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(window_test ${SOURCE_LIST})
set_target_properties(window_test PROPERTIES FOLDER "tests")
target_link_libraries(window_test flame_foundation)
