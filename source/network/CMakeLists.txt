file(GLOB SOURCE_LIST "*.c*")
generate_rc()
add_library(flame_network SHARED ${SOURCE_LIST} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
set_target_properties(flame_network PROPERTIES FOLDER "flame")
target_compile_definitions(flame_network PRIVATE FLAME_NETWORK_MODULE)
target_include_directories(flame_network PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_network flame_foundation)
target_link_libraries(flame_network ws2_32.lib)

