#ifndef DEFERRED
#include "forward.pll"
#else
#include "gbuffer.pll"
#endif

layout(location = 0) out flat uint o_idx;
layout(location = 1) out vec2 o_uv;

void main(void)
{
	uint idx = gl_InstanceIndex >> 16;
	uint tile_idx = gl_InstanceIndex & 0xffff;

	vec2 vs[4] = {
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0),
		vec2(0.0, 1.0)
	};
	uvec2 blocks = terrain_infos[idx].blocks;
	vec2 v = vec2((tile_idx % blocks.x), (tile_idx / blocks.x)) + vs[gl_VertexIndex];
	v /= blocks;

	o_idx = idx;
	o_uv = v;
	gl_Position = vec4(terrain_infos[idx].coord + vec3(v.x, 0.0, v.y) * terrain_infos[idx].extent, 1.0);
}
