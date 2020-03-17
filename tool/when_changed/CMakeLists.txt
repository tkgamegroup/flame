file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(when_changed ${SOURCE_LIST})
set_target_properties(when_changed PROPERTIES FOLDER "tool")
target_link_libraries(when_changed flame_foundation)

