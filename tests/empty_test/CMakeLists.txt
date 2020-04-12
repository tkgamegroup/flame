file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(empty_test ${SOURCE_LIST})
set_target_properties(empty_test PROPERTIES FOLDER "tests")
