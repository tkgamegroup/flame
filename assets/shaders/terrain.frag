#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define RENDER_DATA_SET 0
#define MATERIAL_SET 1
#define LIGHT_SET 2
#define TERRAIN_SET 3

#include "render_data_dsl.glsl"
#include "material_dsl.glsl"
#include "light_dsl.glsl"
#include "terrain_dsl.glsl"
#include "shading.glsl"

layout (location = 0) in flat uint i_idx;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;
layout (location = 5) in vec4 i_debug;

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);

	TerrainInfo terrain = terrain_infos[i_idx];

	vec3 albedo = texture(maps[terrain.color_tex_id], i_uv).rgb;
	
#ifdef WIREFRAME
	o_color = vec4(0.0, 1.0, 0.0, 1.0);
#else
	o_color = vec4(shading(i_coordw, i_coordv, N, V, albedo, vec3(0.0), 1.0), 1.0);
#endif
}
