file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(media_browser "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tools")
target_link_libraries(media_browser flame_worker)
target_link_libraries(media_browser flame_UI)

