file(GLOB_RECURSE source_files "*.c*" "*.asm")
add_executable(empty_test ${source_files})
set_target_properties(empty_test PROPERTIES FOLDER "tests")
#target_link_libraries(empty_test flame_foundation)
