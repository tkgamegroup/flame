#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;
layout (location = 3) in vec3 i_tangent;

struct ObjectMatrix
{
	mat4 mvp;
	mat4 nor;
};

layout (set = 0, binding = 0) buffer ObjectMatrices
{
	ObjectMatrix d[];
}object_matrices;

layout (location = 0) out vec3 o_normal;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	uint mat_idx = gl_InstanceIndex & 0xffff;
	o_normal = vec3(object_matrices.d[mod_idx].nor * vec4(i_normal, 0));
	gl_Position = object_matrices.d[mod_idx].mvp * vec4(i_pos, 1.0);
}
