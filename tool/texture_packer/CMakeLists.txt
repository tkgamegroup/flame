file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(texture_packer "${SOURCE_LIST}" "tool")
target_link_libraries(texture_packer flame_foundation)
