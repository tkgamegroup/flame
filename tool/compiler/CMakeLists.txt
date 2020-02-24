file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(compiler ${SOURCE_LIST})
set_target_properties(compiler PROPERTIES FOLDER "tool")
target_link_libraries(compiler flame_foundation)

