file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(when_changed ${SOURCES})
set_target_properties(when_changed PROPERTIES FOLDER "tools")
target_link_libraries(when_changed flame_foundation)

