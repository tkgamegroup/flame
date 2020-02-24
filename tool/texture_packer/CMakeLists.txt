file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(texture_packer ${SOURCE_LIST})
set_target_properties(texture_packer PROPERTIES FOLDER "tool")
target_link_libraries(texture_packer flame_foundation)
