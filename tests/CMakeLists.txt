file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(test "${SOURCE_LIST}" "tests")
target_link_libraries(test flame_foundation)
target_link_libraries(test flame_graphics)
