file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(8beeper ${SOURCE_LIST})
set_target_properties(8beeper PROPERTIES FOLDER "tests")
target_link_libraries(8beeper flame_sound)
