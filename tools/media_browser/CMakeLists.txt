file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(media_browser "${SOURCE_LIST}" "tools")
target_link_libraries(media_browser flame_worker)
target_link_libraries(media_browser flame_UI)

