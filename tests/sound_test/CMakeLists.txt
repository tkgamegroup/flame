file(GLOB_RECURSE source_files "*.c*")
add_executable(sound_test ${source_files})
set_target_properties(sound_test PROPERTIES FOLDER "tests")
target_link_libraries(sound_test flame_sound)
