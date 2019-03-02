file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(scene_editor "${SOURCE_LIST}" "tools")
target_link_libraries(scene_editor flame_surface)
target_link_libraries(scene_editor flame_system)
target_link_libraries(scene_editor flame_file)
target_link_libraries(scene_editor flame_graphics)
target_link_libraries(scene_editor flame_model)
target_link_libraries(scene_editor flame_UI)

