file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(sound_test ${SOURCE_LIST})
set_target_properties(sound_test PROPERTIES FOLDER "tests")
target_link_libraries(sound_test flame_sound)
