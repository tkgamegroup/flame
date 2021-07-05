file(GLOB_RECURSE source_files "*.c*")
add_executable(universe_test ${source_files})
set_target_properties(universe_test PROPERTIES FOLDER "tests")
target_link_libraries(universe_test flame_universe)
