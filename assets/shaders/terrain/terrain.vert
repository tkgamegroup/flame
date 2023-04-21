layout(location = 0) out vec2 o_uv;

void main(void)
{
	uint terrain_id = pc.index & 0xffff;
	uint block_idx = gl_InstanceIndex;

	vec2 vs[4] = { vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0) };

	uvec2 blocks = instance.terrains[terrain_id].blocks;
	vec2 v = vec2((block_idx % blocks.x), (block_idx / blocks.x)) + vs[gl_VertexIndex];
	v /= blocks;

	o_uv = v;
	gl_Position = instance.terrains[terrain_id].mat * vec4(vec3(v.x, 0.0, v.y) * instance.terrains[terrain_id].extent, 1.0);
}
