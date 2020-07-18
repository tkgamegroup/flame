file(GLOB_RECURSE SOURCE_LIST "*.c*")
set_source_files_properties("test.asm" PROPERTY LANGUAGE ASM_MASM)
add_executable(empty_test ${SOURCE_LIST} "test.asm")
set_target_properties(empty_test PROPERTIES FOLDER "tests")
