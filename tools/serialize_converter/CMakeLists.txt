file(GLOB SOURCE_LIST "src/*.c*")
add_exe(serialize_converter "${SOURCE_LIST}" "tools")
target_include_directories(serialize_converter PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(serialize_converter flame_serialize)

