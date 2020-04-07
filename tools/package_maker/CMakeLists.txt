file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(package_maker ${SOURCE_LIST})
set_target_properties(package_maker PROPERTIES FOLDER "tools")
target_link_libraries(package_maker flame_foundation)
