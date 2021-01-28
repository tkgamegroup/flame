file(GLOB_RECURSE SOURCES "*.c*")
add_executable(wave_maker ${SOURCES})
set_target_properties(wave_maker PROPERTIES FOLDER "tests")
target_link_libraries(wave_maker flame_sound)
