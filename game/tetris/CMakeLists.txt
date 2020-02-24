file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(tetris ${SOURCE_LIST})
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
	"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
	)
set_target_properties(tetris PROPERTIES FOLDER "game")
target_link_libraries(tetris flame_network)
target_link_libraries(tetris flame_sound)
target_link_libraries(tetris flame_universe)
