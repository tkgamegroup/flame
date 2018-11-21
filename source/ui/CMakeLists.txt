file(GLOB SOURCE_LIST "*.c*")
add_lib(flame_ui "${SOURCE_LIST}" "flame")
target_compile_definitions(flame_ui PRIVATE FLAME_UI_MODULE)
target_include_directories(flame_ui PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_ui flame_font)
target_link_libraries(flame_ui flame_graphics)

