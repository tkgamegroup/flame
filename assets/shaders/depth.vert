#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_uv;

struct MeshMatrix
{
	mat4 model;
	mat4 mvp;
	mat4 nor;
};

layout (set = 0, binding = 0) buffer readonly MeshMatrices
{
	MeshMatrix mesh_matrices[];
};

layout (push_constant) uniform PushConstantT
{
	mat4 matrix;
}pc;


layout (location = 0) out flat uint o_mat_id;
layout (location = 1) out vec2 o_uv;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	o_mat_id = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;
	gl_Position = pc.matrix * mesh_matrices[mod_idx].model * vec4(i_pos, 1.0);
}
