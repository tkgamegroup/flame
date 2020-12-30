file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(package_maker ${SOURCES})
set_target_properties(package_maker PROPERTIES FOLDER "tools")
target_link_libraries(package_maker flame_foundation)
