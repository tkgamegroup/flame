file(GLOB_RECURSE SOURCES "*.c*")
add_executable(sound_test ${SOURCES})
set_target_properties(sound_test PROPERTIES FOLDER "tests")
target_link_libraries(sound_test flame_sound)
