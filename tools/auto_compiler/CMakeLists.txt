file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(auto_compiler "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(auto_compiler flame_file)
target_link_libraries(auto_compiler flame_system)
