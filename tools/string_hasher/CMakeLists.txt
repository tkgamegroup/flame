file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(string_hasher ${source_files})
set_target_properties(string_hasher PROPERTIES FOLDER "tools")
target_link_libraries(string_hasher flame_graphics)
