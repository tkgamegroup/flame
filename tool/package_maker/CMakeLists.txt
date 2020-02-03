file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(package_maker "${SOURCE_LIST}" "tool")
target_link_libraries(package_maker flame_foundation)
