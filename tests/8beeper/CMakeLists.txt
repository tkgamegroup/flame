file(GLOB_RECURSE SOURCES "*.c*")
add_executable(8beeper ${SOURCES})
set_target_properties(8beeper PROPERTIES FOLDER "tests")
target_link_libraries(8beeper flame_sound)
