#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_coord;
layout (location = 1) in vec3 i_normal;

struct LightInfo
{
	vec4 col;
	vec4 pos;
};

layout (set = 0, binding = 1) buffer readonly LightInfos
{
	LightInfo light_infos[];
};

struct LightIndices
{
	uint count;
	uint indices[1023];
};

layout (set = 0, binding = 2) buffer readonly LightIndicesList
{
	LightIndices light_indices_list[];
};

layout (set = 0, binding = 3) uniform sampler2D maps[128];

struct MaterialInfo
{
	vec4 albedo_alpha;
	vec4 spec_roughness;
	uint albedo_map_index;
	uint spec_map_index;
	uint roughness_map_index;
	uint normal_map_index;
};

layout (set = 0, binding = 4) uniform MaterialInfos
{
	MaterialInfo material_infos[];
};

layout (location = 0) out vec4 o_color;

void main()
{
	vec3 color = vec3(0);
	uint count = light_indices_list[0].count;
	for (int i = 0; i < count; i++)
	{
		LightInfo light = light_infos[light_indices_list[0].indices[i]];
		vec3 L = light.pos.xyz - i_coord * light.pos.w;
		float dist = length(L);
		L = normalize(L);
		color += dot(i_normal, L) * light.col.rgb / (dist * dist * 0.01);
	}
	o_color = vec4(color, 1.0);
}
