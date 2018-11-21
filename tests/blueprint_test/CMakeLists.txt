file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(blueprint_test "${SOURCE_LIST}" "tests")
target_link_libraries(blueprint_test flame_blueprint)

