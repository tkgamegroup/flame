file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(package_maker ${source_files})
set_target_properties(package_maker PROPERTIES FOLDER "tools")
target_link_libraries(package_maker flame_foundation)
