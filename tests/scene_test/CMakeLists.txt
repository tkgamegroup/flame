file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_lib(scene_test "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tests")
target_link_libraries(scene_test flame_ui)
target_link_libraries(scene_test flame_3d)

