file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(effect_editor "${SOURCE_LIST}" "tools")
target_link_libraries(effect_editor flame_surface)
target_link_libraries(effect_editor flame_system)
target_link_libraries(effect_editor flame_filesystem)
target_link_libraries(effect_editor flame_graphics)
target_link_libraries(effect_editor flame_UI)

