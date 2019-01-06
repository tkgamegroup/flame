file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(window_test "${SOURCE_LIST}" "tests")
target_link_libraries(window_test flame_foundation)
