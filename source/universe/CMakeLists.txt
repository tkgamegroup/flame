if (USE_IMGUI)
	set(source_files "${IMGUI_DIR}/imconfig.h;${IMGUI_DIR}/imgui.h;${IMGUI_DIR}/imgui.cpp;${IMGUI_DIR}/imgui_draw.cpp;${IMGUI_DIR}/imgui_widgets.cpp;${IMGUI_DIR}/imgui_tables.cpp")
	if (USE_IM_FILE_DIALOG)
		set(imfiledialog_files "${IM_FILE_DIALOG_DIR}/ImFileDialog.h;${IM_FILE_DIALOG_DIR}/ImFileDialog.cpp")
	endif()
	add_library(imgui ${source_files} ${imfiledialog_files})
	set_target_properties(imgui PROPERTIES FOLDER "flame")
	target_include_directories(imgui PUBLIC "${IMGUI_DIR}")
	if (USE_IM_FILE_DIALOG)
		target_include_directories(imgui PUBLIC "${IM_FILE_DIALOG_DIR}")
		target_include_directories(imgui PRIVATE "${STB_INCLUDE_DIR}")
	endif()
endif()

file(GLOB_RECURSE source_files "*.h*" "*.c*")
file(GLOB_RECURSE script_files "${ASSETS_DIR}/*.lua")
file(GLOB_RECURSE prefab_files "${ASSETS_DIR}/*.prefab")
file(GLOB_RECURSE shader_files 
	"${SHADERS_DIR}/*.glsl"
	"${SHADERS_DIR}/*.vert"
	"${SHADERS_DIR}/*.tesc"
	"${SHADERS_DIR}/*.tese"
	"${SHADERS_DIR}/*.geom"
	"${SHADERS_DIR}/*.frag"
	"${SHADERS_DIR}/*.comp"
	"${SHADERS_DIR}/*.dsl"
	"${SHADERS_DIR}/*.pll"
	"${SHADERS_DIR}/*.pipeline"
	"${SHADERS_DIR}/*.rp"
)
add_library(flame_universe SHARED ${source_files} ${script_files} ${prefab_files} ${shader_files})
assign_source_group("${CMAKE_CURRENT_SOURCE_DIR}" "" ${source_files})
assign_source_group("${ASSETS_DIR}" "scripts\\" ${script_files})
assign_source_group("${ASSETS_DIR}" "prefabs\\" ${prefab_files})
assign_source_group("${SHADERS_DIR}" "shaders\\" ${shader_files})
set_target_properties(flame_universe PROPERTIES FOLDER "flame")
target_compile_definitions(flame_universe PRIVATE FLAME_UNIVERSE_MODULE)
if (USE_IMGUI)
	target_compile_definitions(flame_universe PUBLIC USE_IMGUI)
	target_link_libraries(flame_universe imgui)
	if (USE_IM_FILE_DIALOG)
		target_compile_definitions(flame_universe PUBLIC USE_IM_FILE_DIALOG)
	endif()
endif()
target_link_libraries(flame_universe flame_network)
target_link_libraries(flame_universe flame_database)
target_link_libraries(flame_universe flame_graphics)
target_link_libraries(flame_universe flame_sound)
target_link_libraries(flame_universe flame_physics)
target_link_libraries(flame_universe flame_script)
target_include_directories(flame_universe PRIVATE "${SHADERS_DIR}")
add_dependencies(flame_universe typeinfogen_dep)
add_custom_command(TARGET flame_universe POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> -i $<TARGET_FILE:flame_universe> -d "${CMAKE_CURRENT_SOURCE_DIR}/typeinfo.desc")

file(GLOB PUBLIC_HEADER_FILES "*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe")

file(GLOB PUBLIC_HEADER_FILES "components/*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe/components")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe/components")

file(GLOB PUBLIC_HEADER_FILES "systems/*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe/systems")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe/systems")