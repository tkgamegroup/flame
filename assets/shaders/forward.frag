#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_normal;

struct LightInfo
{
	vec4 col;
	vec4 pos;
};

layout (set = 0, binding = 1) buffer readonly LightInfos
{
	LightInfo light_infos[];
};

layout (set = 0, binding = 2) uniform sampler2D maps[128];

struct MaterialInfo
{
	vec4 albedo;
	vec4 spec_roughness;
	uint albedo_map_index;
	uint spec_map_index;
	uint roughness_map_index;
	uint normal_map_index;
};

layout (set = 0, binding = 3) uniform MaterialInfos
{
	MaterialInfo material_infos[];
};

layout (location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(vec3(dot(i_normal, vec3(0, 0, 1))) * 2.0, 1.0);
}
