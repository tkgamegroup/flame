file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(code_solver "${SOURCE_LIST}" "tools")
target_link_libraries(code_solver flame_foundation)
