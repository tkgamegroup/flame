file(GLOB_RECURSE SOURCES "*.c*")
add_executable(universe_test ${SOURCES})
set_target_properties(universe_test PROPERTIES FOLDER "tests")
target_link_libraries(universe_test flame_universe)
