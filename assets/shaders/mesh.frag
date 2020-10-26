#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define RENDER_DATA_SET 0
#define MATERIAL_SET 2
#define LIGHT_SET 3

#include "render_data_dsl.glsl"
#include "material_dsl.glsl"
#include "light_dsl.glsl"
#include "shading.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coordw;
layout (location = 3) in vec3 i_coordv;
layout (location = 4) in vec3 i_normal;
layout (location = 5) in vec4 i_debug;

layout (location = 0) out vec4 o_color;

void main()
{
	vec3 res = vec3(0.0);

	MaterialInfo material = material_infos[i_mat_id];

	vec4 color;
	if (material.color_map_index >= 0)
		color = texture(maps[material.color_map_index], i_uv);
	else
		color = material.color;

	if (color.a < material.alpha_test)
		discard;

	float metallic;
	float roughness;
	if (material.metallic_roughness_ao_map_index >= 0)
	{
		vec4 s = texture(maps[material.metallic_roughness_ao_map_index], i_uv);
		metallic = s.r;
		roughness = s.g;
	}
	else
	{
		metallic = material.metallic;
		roughness = material.roughness;
	}
	vec3 albedo = (1.0 - metallic) * color.rgb;
	vec3 spec = mix(vec3(0.04), color.rgb, metallic);

	vec3 N = normalize(i_normal);
	vec3 V = normalize(i_coordv);
	
	o_color = vec4(shading(i_coordw, i_coordv, N, V, albedo, spec, roughness), 1.0);
}
