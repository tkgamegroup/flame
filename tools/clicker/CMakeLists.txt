file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(clicker ${SOURCES})
set_target_properties(clicker PROPERTIES FOLDER "tools")
target_link_libraries(clicker flame_foundation)

