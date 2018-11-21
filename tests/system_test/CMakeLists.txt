file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(system_test "${SOURCE_LIST}" "tests")
target_include_directories(system_test PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(system_test flame_memory)
#target_link_libraries(system_test flame_system)

