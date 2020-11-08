#version 450 core
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_separate_shader_objects : enable

#define TERRAIN_SET 3

#include "terrain.dsl"

layout (location = 0) out flat uint o_idx;
layout (location = 1) out vec2 o_uv;

void main(void)
{
	uint idx = gl_InstanceIndex >> 16;
	uint tile_idx = gl_InstanceIndex & 0xffff;
	TerrainInfo terrain = terrain_infos[idx];

	vec2 vs[4] = {
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0),
		vec2(0.0, 1.0)
	};
	vec2 v = vec2((tile_idx % terrain.blocks.x) + vs[gl_VertexIndex].x, (tile_idx / terrain.blocks.x) + vs[gl_VertexIndex].y);

	gl_Position = vec4(vec3(v.x * terrain.scale.x, 0.0, v.y * terrain.scale.z) + terrain.coord, 1.0);
	o_idx = idx;
	o_uv = v / terrain.blocks;
}
