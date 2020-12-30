file(GLOB_RECURSE SOURCES "*.h*" "*.c*")
add_executable(texture_packer ${SOURCES})
set_target_properties(texture_packer PROPERTIES FOLDER "tools")
target_link_libraries(texture_packer flame_foundation)
