file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(sound_recorder "${SOURCE_LIST}" "tool")
target_link_libraries(sound_recorder flame_sound)

