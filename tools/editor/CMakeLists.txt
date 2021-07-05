generate_config()
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/scene_editor_layout.xml)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/scene_editor_layout.xml
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
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/bp_editor_layout.xml)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bp_editor_layout.xml
		"<layout>\n"
		"	<static>\n"
		"		<layout type=\"h\">\n"
		"			<layout type=\"v\" r=\"2\">\n"
		"				<docker>\n"
		"					<page name=\"detail\" />\n"
		"				</docker>\n"
		"				<docker>\n"
		"					<page name=\"preview\" />\n"
		"				</docker>\n"
		"			</layout>\n"
		"			<docker r=\"8\">\n"
		"				<page name=\"editor\" />\n"
		"			</docker>\n"
		"		</layout>\n"
		"	</static>\n"
		"	<floating>\n"
		"	</floating>\n"
		"</layout>\n")
endif()
file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(editor ${source_files})
set_target_properties(editor PROPERTIES FOLDER "tools")
target_compile_definitions(editor PRIVATE GRAPHVIZ_PATH="${GRAPHVIZ_PATH}")
target_link_libraries(editor flame_universe)
add_dependencies(editor typeinfogen_dep)

