file(GLOB HEADER_LIST "src/*.h*")
file(GLOB SOURCE_LIST "src/*.c*")
add_exe(typeinfogen "${HEADER_LIST}" "${SOURCE_LIST}" "/src" "tools")
target_include_directories(typeinfogen PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(typeinfogen flame_typeinfo)

