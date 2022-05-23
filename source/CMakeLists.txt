add_subdirectory(foundation)
add_subdirectory(typeinfogen)

# build imgui by ourselves because we need imgui to include glm and we need to add some extensions imgui
if(USE_IMGUI)
	set(config_file "${CMAKE_CURRENT_SOURCE_DIR}/imgui_config.h")
	set(source_files "${config_file};${CMAKE_CURRENT_SOURCE_DIR}/imgui_extension.h;${CMAKE_CURRENT_SOURCE_DIR}/imgui_extension.cpp;${IMGUI_DIR}/imgui.h;${IMGUI_DIR}/imgui.cpp;${IMGUI_DIR}/imgui_draw.cpp;${IMGUI_DIR}/imgui_widgets.cpp;${IMGUI_DIR}/imgui_tables.cpp;${IMGUI_DIR}/misc/cpp/imgui_stdlib.h;${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp")
	if(USE_IM_GUIZMO)
		set(imguizmo_files "${IM_GUIZMO_DIR}/ImGuizmo.h;${IM_GUIZMO_DIR}/ImGuizmo.cpp")
	endif()
	add_library(imgui ${source_files} ${imfiledialog_files} ${imguizmo_files})
	target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="${config_file}")
	target_include_directories(imgui PUBLIC "${IMGUI_DIR}")
	target_include_directories(imgui PRIVATE "${GLM_INCLUDE_DIR}")
	if(USE_IM_GUIZMO)
		target_compile_definitions(imgui PUBLIC USE_IM_GUIZMO)
		target_include_directories(imgui PUBLIC "${IM_GUIZMO_DIR}")
	endif()

	file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame")
	add_custom_command(TARGET imgui PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/include/flame")
	add_custom_command(TARGET imgui PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/imgui_extension.h" "${CMAKE_SOURCE_DIR}/include/flame")
endif()

target_link_libraries(imgui flame_foundation)

add_subdirectory(graphics)
add_subdirectory(universe)
