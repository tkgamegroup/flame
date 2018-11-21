project(editor)

file(GLOB_RECURSE EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EDITOR_SOURCE_LIST "src/*.c*")

group_source("${EDITOR_HEADER_LIST}" "/src" "Header")
group_source("${EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(editor ${EDITOR_HEADER_LIST} ${EDITOR_SOURCE_LIST})

target_link_libraries(editor flame_engine)

set_target_properties(editor PROPERTIES FOLDER "tools/editor") 
set_target_properties(editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")


set_output_dir("${CMAKE_SOURCE_DIR}/bin/ui/windows")

file(GLOB_RECURSE EDITOR_WINDOWS_SOURCE_LIST "windows/*.c*")

foreach(f ${EDITOR_WINDOWS_SOURCE_LIST})
	get_filename_component(ff "${f}" NAME_WE)
	set(target_name editor_window_${ff})
	add_library(${target_name} SHARED ${f})
	target_link_libraries(${target_name} flame_engine)
	set_target_properties(${target_name} PROPERTIES FOLDER "tools/editor/windows") 
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endforeach()

set_output_dir("${CMAKE_SOURCE_DIR}/bin")