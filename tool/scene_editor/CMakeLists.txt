file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(scene_editor ${SOURCE_LIST})
set_target_properties(scene_editor PROPERTIES FOLDER "tool")
set_target_properties(scene_editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(scene_editor flame_foundation)
target_link_libraries(scene_editor flame_graphics)
target_link_libraries(scene_editor flame_universe)

