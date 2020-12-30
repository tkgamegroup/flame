file(GLOB_RECURSE SOURCES "*.c*")
add_executable(window_test ${SOURCES})
set_target_properties(window_test PROPERTIES FOLDER "tests")
target_link_libraries(window_test flame_foundation)
