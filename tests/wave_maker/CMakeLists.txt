file(GLOB_RECURSE source_files "*.c*")
add_executable(wave_maker ${source_files})
set_target_properties(wave_maker PROPERTIES FOLDER "tests")
target_link_libraries(wave_maker flame_sound)
