generate_config()
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml
		"<layout>\n"
		"	<static>\n"
		"		<layout type=\"h\">\n"
		"			<layout type=\"v\" r=\"3\">\n"
		"				<docker>\n"
		"					<page name=\"detail\" />\n"
		"				</docker>\n"
		"				<docker>\n"
		"					<page name=\"preview\" />\n"
		"				</docker>\n"
		"			</layout>\n"
		"			<docker r=\"7\">\n"
		"				<page name=\"editor\" />\n"
		"			</docker>\n"
		"		</layout>\n"
		"	</static>\n"
		"	<floating>\n"
		"	</floating>\n"
		"</layout>\n")
endif()
file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(bp_editor ${SOURCE_LIST})
set_target_properties(bp_editor PROPERTIES FOLDER "tools")
target_compile_definitions(bp_editor PRIVATE GRAPHVIZ_PATH="${GRAPHVIZ_PATH}")
target_link_libraries(bp_editor flame_foundation)
target_link_libraries(bp_editor flame_graphics)
target_link_libraries(bp_editor flame_universe)
target_link_libraries(bp_editor flame_sound)
add_dependencies(bp_editor typeinfogen)
