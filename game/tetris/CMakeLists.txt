file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(tetris "${SOURCE_LIST}" "game")
target_link_libraries(tetris flame_ui)
target_link_libraries(tetris flame_sound)
