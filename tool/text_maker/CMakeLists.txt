file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(text_maker ${SOURCE_LIST})
set_target_properties(text_maker PROPERTIES FOLDER "tool")
target_link_libraries(text_maker flame_foundation)
