file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(when_key ${SOURCES})
set_target_properties(when_key PROPERTIES FOLDER "tools")
target_link_libraries(when_key flame_foundation)

