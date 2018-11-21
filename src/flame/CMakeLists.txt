add_lib(flame_memory "memory.h" "memory.cpp" "" "flame")
target_compile_definitions(flame_memory PRIVATE FLAME_MEMORY_MODULE)
target_include_directories(flame_memory PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_lib(flame_function "function.h" "function.cpp" "" "flame")
target_compile_definitions(flame_function PRIVATE FLAME_FUNCTION_MODULE)
target_include_directories(flame_function PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_lib(flame_file "file.h" "file.cpp" "" "flame")
target_compile_definitions(flame_file PRIVATE FLAME_FILE_MODULE)
target_include_directories(flame_file PRIVATE ${RAPIDXML_PATH})
target_include_directories(flame_file PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_lib(flame_bitmap "bitmap.h" "bitmap.cpp" "" "flame")
target_compile_definitions(flame_bitmap PRIVATE FLAME_BITMAP_MODULE)
target_include_directories(flame_bitmap PRIVATE ${STB_PATH})
target_include_directories(flame_bitmap PUBLIC "${CMAKE_SOURCE_DIR}/src")

add_lib(flame_font "font.h" "font.cpp" "" "flame")
target_compile_definitions(flame_font PRIVATE FLAME_FONT_MODULE)
target_include_directories(flame_font PRIVATE ${FREETYPE_PATH}/include)
target_include_directories(flame_font PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(flame_font ${FREETYPE_PATH}/win64/freetype.lib)
target_link_libraries(flame_font flame_file)
target_link_libraries(flame_font flame_bitmap)
add_custom_target(copy_freetype_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_PATH}/win64/freetype.dll ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_font copy_freetype_dlls)

add_lib(flame_system "system.h" "system.cpp" "" "flame")
target_compile_definitions(flame_system PRIVATE FLAME_SYSTEM_MODULE)
target_include_directories(flame_system PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(flame_system flame_memory)
target_link_libraries(flame_system flame_function)

add_lib(flame_worker "worker.h" "worker.cpp" "" "flame")
target_compile_definitions(flame_worker PRIVATE FLAME_WORKER_MODULE)
target_include_directories(flame_worker PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(flame_worker flame_function)

add_lib(flame_window "window.h" "window.cpp" "" "flame")
target_compile_definitions(flame_window PRIVATE FLAME_WINDOW_MODULE)
target_include_directories(flame_window PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(flame_window flame_file)
target_link_libraries(flame_window flame_system)
target_link_libraries(flame_window flame_bitmap)

add_lib(flame_typeinfo "typeinfo.h" "typeinfo.cpp" "" "flame")
target_compile_definitions(flame_typeinfo PRIVATE FLAME_TYPEINFO_MODULE)
target_include_directories(flame_typeinfo PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_typeinfo PUBLIC "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/include")
target_include_directories(flame_typeinfo PUBLIC "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Tools/MSVC/14.14.26428/atlmfc/include")
target_link_libraries(flame_typeinfo "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/lib/amd64/diaguids.lib")
target_link_libraries(flame_typeinfo flame_file)
target_link_libraries(flame_typeinfo flame_system)
add_custom_target(copy_dia_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/DIA SDK/bin/amd64/msdia140.dll" ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_typeinfo copy_dia_dlls)

add_lib(flame_blueprint "blueprint.h" "blueprint.cpp" "" "flame")
target_compile_definitions(flame_blueprint PRIVATE FLAME_BLUEPRINT_MODULE)
target_include_directories(flame_blueprint PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(flame_blueprint flame_file)
target_link_libraries(flame_blueprint flame_typeinfo)

add_subdirectory(graphics)
add_subdirectory(ui)
add_subdirectory(3d)
if (FLAME_ENABLE_PHYSICS)
add_subdirectory(physics)
endif()
add_subdirectory(sound)