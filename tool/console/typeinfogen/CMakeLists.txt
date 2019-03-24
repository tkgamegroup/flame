file(GLOB SOURCE_LIST "*.c*")
add_exe(typeinfogen "${SOURCE_LIST}" "tool")
target_link_libraries(typeinfogen flame_foundation)

