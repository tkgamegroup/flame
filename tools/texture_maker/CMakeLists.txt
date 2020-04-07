file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(texture_maker ${SOURCE_LIST})
set_target_properties(texture_maker PROPERTIES FOLDER "tools")
target_link_libraries(texture_maker flame_foundation)
