file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_lib(scene_test "${SOURCE_LIST}" "tests")
target_link_libraries(scene_test flame_ui)
target_link_libraries(scene_test flame_3d)

