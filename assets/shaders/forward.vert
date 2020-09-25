#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define MESH_SET 0
#define RENDER_DATA_SET 3

#include "mesh_dsl.glsl"
#include "render_data_dsl.glsl"

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec2 i_uv;
layout (location = 2) in vec3 i_normal;
//layout (location = 3) in vec3 i_tangent;

layout (location = 0) out flat uint o_mat_id;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_coordw;
layout (location = 3) out vec3 o_coordv;
layout (location = 4) out vec3 o_normal;
layout (location = 5) out vec4 o_debug;

void main()
{
	uint mod_idx = gl_InstanceIndex >> 16;
	o_mat_id = gl_InstanceIndex & 0xffff;
	o_uv = i_uv;
	o_coordw = vec3(mesh_matrices[mod_idx].model * vec4(i_position, 1.0));
	o_coordv = render_data.camera_coord - o_coordw;
	o_normal = vec3(mesh_matrices[mod_idx].normal * vec4(i_normal, 0));
	gl_Position = render_data.proj_view * vec4(o_coordw, 1.0);
}
