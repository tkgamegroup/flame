file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(clicker ${SOURCE_LIST})
set_target_properties(clicker PROPERTIES FOLDER "tools")
target_link_libraries(clicker flame_foundation)

