#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec3 i_coord;
layout (location = 2) in vec3 i_normal;
layout (location = 3) in vec2 i_uv;

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
	bool conductor;
	vec4 color;
	float roughness;
	float alpha_test;
	int color_map_index;
	int normal_roughness_map_index;
};

layout (set = 0, binding = 4) uniform MaterialInfos
{
	MaterialInfo material_infos[128];
};

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(0);

	MaterialInfo material = material_infos[i_mat_id];
	vec4 color;
	if (material.color_map_index >= 0)
		color = texture(maps[material.color_map_index], i_uv);
	else
		color = material.color;

	if (color.a < material.alpha_test)
		discard;

	uint count = light_indices_list[0].count;
	for (int i = 0; i < count; i++)
	{
		LightInfo light = light_infos[light_indices_list[0].indices[i]];
		vec3 l = light.pos.xyz - i_coord * light.pos.w;
		float dist = length(l);
		l = l / dist;
		vec3 Li = dot(i_normal, l) * light.col.rgb / (dist * dist * 0.01);

		if (material.conductor)
		{
		}
		else
		{
			o_color.rgb += color.rgb * Li;
		}
	}

	o_color.a = color.a;
}
