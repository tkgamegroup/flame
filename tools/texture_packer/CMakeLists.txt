file(GLOB_RECURSE source_files "*.h*" "*.c*")
add_executable(texture_packer ${source_files})
set_target_properties(texture_packer PROPERTIES FOLDER "tools")
target_link_libraries(texture_packer flame_graphics)
