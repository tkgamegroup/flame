#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_coord;

struct MaterialInfo
{
	vec4 color;
	float metallic;
	float roughness;
	float alpha_test;
	float dummy0;
	int color_map_index;
	int metallic_roughness_ao_map_index;
	int normal_height_map_index;
	int dummy1;
};

layout (set = 0, binding = 1) buffer readonly MaterialInfos
{
	MaterialInfo material_infos[];
};

layout (set = 0, binding = 2) uniform sampler2D maps[128];

layout (location = 0) out float o_color;

void main()
{
	MaterialInfo material = material_infos[i_mat_id];

	vec4 color;
	if (material.color_map_index >= 0)
		color = texture(maps[material.color_map_index], i_uv);
	else
		color = material.color;

	if (color.a < material.alpha_test)
		discard;

	o_color = length(i_coord);
}
