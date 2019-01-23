file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_exe(compiler "${SOURCE_LIST}" "tools")
target_compile_definitions(compiler PRIVATE VS_LOCATION="${VS_PATH}")
target_link_libraries(compiler flame_foundation)
