file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(communicator ${source_files})
set_target_properties(communicator PROPERTIES FOLDER "tools")
target_link_libraries(communicator flame_foundation)
