file(GLOB SOURCE_LIST "*.c*")
add_exe(typeinfogen "${SOURCE_LIST}" "tools")
target_compile_definitions(typeinfogen PRIVATE VS_LOCATION="${VS_PATH}")
target_link_libraries(typeinfogen flame_foundation)

