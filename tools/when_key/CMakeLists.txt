file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(when_key ${SOURCE_LIST})
set_target_properties(when_key PROPERTIES FOLDER "tools")
target_link_libraries(when_key flame_foundation)

