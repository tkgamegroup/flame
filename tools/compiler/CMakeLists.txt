file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(compiler ${SOURCE_LIST})
set_target_properties(compiler PROPERTIES FOLDER "tools")
target_link_libraries(compiler flame_foundation)

