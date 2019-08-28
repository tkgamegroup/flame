file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(tetris "${SOURCE_LIST}" "game")
target_link_libraries(tetris flame_universe)
