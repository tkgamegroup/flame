file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(system_test "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tests")
target_link_libraries(system_test flame_system)

