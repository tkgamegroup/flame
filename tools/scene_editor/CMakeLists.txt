generate_config()
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/window_layout.xml
		"<layout>\n"
		"	<static>\n"
		"		<layout type=\"h\">\n"
		"			<layout type=\"v\" r=\"8\">\n"
		"				<docker r=\"7\">\n"
		"					<page name=\"editor\" />\n"
		"				</docker>\n"
		"				<docker r=\"3\">\n"
		"					<page name=\"resource_explorer\" />\n"
		"				</docker>\n"
		"			</layout>\n"
		"			<layout type=\"v\" r=\"2\">\n"
		"				<docker r=\"2\">\n"
		"					<page name=\"hierarchy\" />\n"
		"				</docker>\n"
		"				<docker r=\"8\">\n"
		"					<page name=\"inspector\" />\n"
		"				</docker>\n"
		"			</layout>\n"
		"		</layout>\n"
		"	</static>\n"
		"	<floating>\n"
		"	</floating>\n"
		"</layout>\n")
endif()
file(GLOB_RECURSE SOURCE_LIST "*.h*" "*.c*")
add_executable(scene_editor ${SOURCE_LIST})
set_target_properties(scene_editor PROPERTIES FOLDER "tools")
target_link_libraries(scene_editor flame_universe)
add_dependencies(scene_editor typeinfogen)

