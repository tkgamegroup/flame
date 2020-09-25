#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define TERRAIN_SET 3

#include "terrain_dsl.glsl"

layout (location = 0) out vec2 o_uv;

void main(void)
{
	vec2 vs[4] = {
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0),
		vec2(0.0, 1.0)
	};
	vec2 v = vec2((gl_InstanceIndex % terrain_info.size.x) + vs[gl_VertexIndex].x, (gl_InstanceIndex / terrain_info.size.x) + vs[gl_VertexIndex].y);
	gl_Position = vec4(vec3(v.x * terrain_info.extent.x, 0.0, v.y * terrain_info.extent.z) + terrain_info.coord, 1.0);
	o_uv = v / terrain_info.size;
}
