file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
generate_rc()
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
add_executable(bp_editor ${SOURCE_LIST} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
set_target_properties(bp_editor PROPERTIES FOLDER "tool")
target_compile_definitions(bp_editor PRIVATE GRAPHVIZ_PATH="${GRAPHVIZ_PATH}")
target_link_libraries(bp_editor flame_foundation)
target_link_libraries(bp_editor flame_graphics)
target_link_libraries(bp_editor flame_universe)
target_link_libraries(bp_editor flame_sound)
