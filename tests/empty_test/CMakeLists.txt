file(GLOB_RECURSE SOURCES "*.c*" "*.asm")
add_executable(empty_test ${SOURCES})
set_target_properties(empty_test PROPERTIES FOLDER "tests")
