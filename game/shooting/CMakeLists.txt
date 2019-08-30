file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(shooting "${SOURCE_LIST}" "game")
target_link_libraries(shooting flame_universe)
