add_lib(flame_memory "memory.cpp" "flame")
target_compile_definitions(flame_memory PRIVATE FLAME_MEMORY_MODULE)
target_include_directories(flame_memory PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_lib(flame_function "function.cpp" "flame")
target_compile_definitions(flame_function PRIVATE FLAME_FUNCTION_MODULE)
target_include_directories(flame_function PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_lib(flame_serialize "serialize.cpp" "flame")
target_compile_definitions(flame_serialize PRIVATE FLAME_SERIALIZE_MODULE)
target_include_directories(flame_serialize PRIVATE ${RAPIDXML_PATH})
target_include_directories(flame_serialize PRIVATE "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/include")
target_include_directories(flame_serialize PRIVATE "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.14.26428/atlmfc/include")
target_include_directories(flame_serialize PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_serialize "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/lib/amd64/diaguids.lib")
target_link_libraries(flame_serialize flame_memory)
target_link_libraries(flame_serialize flame_function)
add_custom_target(copy_dia_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/bin/amd64/msdia140.dll" ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_serialize copy_dia_dlls)

add_lib(flame_bitmap "bitmap.cpp" "flame")
target_compile_definitions(flame_bitmap PRIVATE FLAME_BITMAP_MODULE)
target_include_directories(flame_bitmap PRIVATE ${STB_PATH})
target_include_directories(flame_bitmap PUBLIC "${CMAKE_SOURCE_DIR}/include")

add_lib(flame_system "system.cpp" "flame")
target_compile_definitions(flame_system PRIVATE FLAME_SYSTEM_MODULE)
target_include_directories(flame_system PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_system flame_memory)
target_link_libraries(flame_system flame_function)

add_lib(flame_worker "worker.cpp" "flame")
target_compile_definitions(flame_worker PRIVATE FLAME_WORKER_MODULE)
target_include_directories(flame_worker PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_worker flame_function)

add_lib(flame_font "font.cpp" "flame")
target_compile_definitions(flame_font PRIVATE FLAME_FONT_MODULE)
target_include_directories(flame_font PRIVATE ${FREETYPE_PATH}/include)
target_include_directories(flame_font PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_font ${FREETYPE_PATH}/win64/freetype.lib)
target_link_libraries(flame_font flame_serialize)
target_link_libraries(flame_font flame_bitmap)
add_custom_target(copy_freetype_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_PATH}/win64/freetype.dll ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_font copy_freetype_dlls)

add_lib(flame_window "window.cpp" "flame")
target_compile_definitions(flame_window PRIVATE FLAME_WINDOW_MODULE)
target_include_directories(flame_window PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_window flame_serialize)
target_link_libraries(flame_window flame_system)
target_link_libraries(flame_window flame_bitmap)

add_lib(flame_blueprint "blueprint.cpp" "flame")
target_compile_definitions(flame_blueprint PRIVATE FLAME_BLUEPRINT_MODULE)
target_include_directories(flame_blueprint PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(flame_blueprint flame_serialize)

add_subdirectory(graphics)
add_subdirectory(ui)
add_subdirectory(3d)
if (FLAME_ENABLE_PHYSICS)
add_subdirectory(physics)
endif()
add_subdirectory(sound)