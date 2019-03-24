file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(test "${SOURCE_LIST}" "test")
target_link_libraries(test flame_universe)