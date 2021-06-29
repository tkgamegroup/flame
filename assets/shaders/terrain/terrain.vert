#ifndef DEFERRED
#include "forward.pll"
#else
#include "gbuffer.pll"
#endif

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
	vec2 v = vec2((tile_idx % terrain.blocks.x), (tile_idx / terrain.blocks.x)) + vs[gl_VertexIndex];
	v /= terrain.blocks;

	gl_Position = vec4(terrain.coord + vec3(v.x, 0.0, v.y) * terrain.extent, 1.0);
	o_idx = idx;
	o_uv = v;
}
