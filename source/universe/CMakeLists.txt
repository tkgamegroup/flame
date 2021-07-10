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
	"${SHADERS_DIR}/*.pl"
	"${SHADERS_DIR}/*.rp"
)
add_library(flame_universe SHARED ${source_files} ${script_files} ${prefab_files} ${shader_files})
assign_source_group("${CMAKE_CURRENT_SOURCE_DIR}" "" ${source_files})
assign_source_group("${ASSETS_DIR}" "scripts\\" ${script_files})
assign_source_group("${ASSETS_DIR}" "prefabs\\" ${prefab_files})
assign_source_group("${SHADERS_DIR}" "shaders\\" ${shader_files})
set_target_properties(flame_universe PROPERTIES FOLDER "flame")
set_target_properties(flame_universe PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
target_compile_definitions(flame_universe PRIVATE FLAME_UNIVERSE_MODULE)
target_link_libraries(flame_universe flame_network)
target_link_libraries(flame_universe flame_database)
target_link_libraries(flame_universe flame_graphics)
target_link_libraries(flame_universe flame_sound)
target_link_libraries(flame_universe flame_physics)
target_link_libraries(flame_universe flame_script)
target_include_directories(flame_universe PRIVATE "${SHADERS_DIR}")
add_dependencies(flame_universe typeinfogen_dep)
add_custom_command(TARGET flame_universe POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> -i $<TARGET_FILE:flame_universe> -d "${CMAKE_CURRENT_SOURCE_DIR}/typeinfo.desc")
add_dependencies(flame_universe shader_compiler)
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/element/element.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/mesh/forward.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/mesh/gbuffer.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/terrain/forward.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/terrain/gbuffer.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/deferred/deferred.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/particle/particle.pll")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND $<TARGET_FILE:shader_compiler> "${SHADERS_DIR}/post/post.pll")

file(GLOB PUBLIC_HEADER_FILES "*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe")

file(GLOB PUBLIC_HEADER_FILES "components/*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe/components")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe/components")

file(GLOB PUBLIC_HEADER_FILES "drivers/*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe/drivers")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe/drivers")

file(GLOB PUBLIC_HEADER_FILES "systems/*.h*")
list(FILTER PUBLIC_HEADER_FILES EXCLUDE REGEX ".*_private[.]h$")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/flame/universe/systems")
add_custom_command(TARGET flame_universe PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PUBLIC_HEADER_FILES} "${CMAKE_SOURCE_DIR}/include/flame/universe/systems")
