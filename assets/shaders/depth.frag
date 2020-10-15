#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define MATERIAL_SET 1

#include "material_dsl.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;

layout (push_constant) uniform PushConstantT
{
	mat4 proj_view;
	float zNear;
	float zFar;
}pc;

layout (location = 0) out float o_depth;

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

	if (pc.zNear == 0.0)
		o_depth = gl_FragCoord.z;
	else
		o_depth = pc.zNear / (pc.zFar + gl_FragCoord.z * (pc.zNear - pc.zFar));
}
