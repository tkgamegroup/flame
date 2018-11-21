file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(system_test "${SOURCE_LIST}" "/src" "tests")
target_link_libraries(system_test flame_system)

