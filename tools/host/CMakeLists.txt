file(GLOB SOURCE_LIST "src/*.c*")
add_exe(host "${SOURCE_LIST}" "/src" "tools")
target_include_directories(host PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(host flame_ui)
target_link_libraries(host flame_3d)

