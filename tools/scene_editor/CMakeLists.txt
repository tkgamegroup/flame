file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml
		"<layout>\n"
		"	<static>\n"
		"		<editor />\n"
		"	</static>\n"
		"	<floating>\n"
		"	</floating>\n"
		"</layout>\n")
endif()
add_executable(scene_editor ${SOURCE_LIST})
set_target_properties(scene_editor PROPERTIES FOLDER "tools")
target_link_libraries(scene_editor flame_foundation)
target_link_libraries(scene_editor flame_graphics)
target_link_libraries(scene_editor flame_universe)
target_link_libraries(scene_editor flame_sound)
add_dependencies(scene_editor typeinfogen)
add_dependencies(scene_editor compiler)

