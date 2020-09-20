#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#include "material_dsl.glsl"
#include "linear_depth.glsl"

layout (location = 0) in flat uint i_mat_id;
layout (location = 1) in vec2 i_uv;

layout (push_constant) uniform PushConstantT
{
	mat4 matrix;
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

	float z = gl_FragCoord.z;
	if (pc.zNear == 0.0)
		o_depth = z;
	else
		o_depth = linear_depth(z, pc.zNear, pc.zFar);
}
