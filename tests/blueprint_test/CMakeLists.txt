file(GLOB_RECURSE HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SOURCE_LIST "src/*.c*")
add_exe(blueprint_test "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tests")
target_link_libraries(blueprint_test flame_blueprint)

