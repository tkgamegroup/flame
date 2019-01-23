file(GLOB SOURCE_LIST "*.c*")
add_exe(typeinfogen "${SOURCE_LIST}" "tools")
target_include_directories(typeinfogen PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(typeinfogen flame_foundation)

