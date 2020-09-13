#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;
//layout (location = 3) in vec3 i_tangent;

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

layout (set = 1, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 proj;
	mat4 proj_view;
	vec4 coord;
	vec4 dummy0;
	vec4 dummy1;
	vec4 dummy2;
}camera_data;

layout (location = 0) out flat uint o_mat_id;
layout (location = 1) out vec3 o_coordw;
layout (location = 2) out vec3 o_coordv;
layout (location = 3) out vec3 o_normal;
layout (location = 4) out vec2 o_uv;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	o_mat_id = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;
	o_coordw = vec3(mesh_matrices[mod_idx].model * vec4(i_pos, 1.0));
	o_coordv = vec3(camera_data.coord) - o_coordw;
	o_normal = vec3(mesh_matrices[mod_idx].nor * vec4(i_normal, 0));
	gl_Position = mesh_matrices[mod_idx].mvp * vec4(i_pos, 1.0);
}
